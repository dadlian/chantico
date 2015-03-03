<?php
	class MySQLConnection extends DatabaseConnection{
		/*
		 * Database Host
		 */
		/** @String(required=true, default='localhost') */
		protected $hostname;
		
		/*
		 * Database User
		 */
		/** @String(required=true, default='root') */
		protected $username;
		
		/*
		 * Database Password
		 */
		/** @String(required=true) */
		protected $password;
		
		/*
		 * Database Name
		 */
		/** @String(required=true) */
		protected $database;
		
		/*
		 * Database Connection
		 */
		private $connection;
		
		public function __construct(){
			call_user_func_array(array("parent", "__construct"), func_get_args());
			$this->connect();
		}
	
		/*
		 * Establishes a connection to a MySQL database server. Closes an existing connection if any.
		 */
		public function connect(){			
			if($this->connection){
				return;
			}
			
			@$newConnection = new mysqli($this->getHostname(), $this->getUsername(), $this->getPassword(), $this->getDatabase());
			
			if($newConnection->connect_error){
				fatal_error(DATABASE_CONNECT_ERROR, $newConnection->connect_error.".");
				return;
			}
			
			$this->connection = $newConnection;
			$this->connection->autocommit(FALSE);
			$this->connection->query("SET NAMES 'utf8'");
		}
		
		/*
		 * Returns a result set after executing the passed statement on the database connection. 
		 * Any additional arguments are bound to the prepared statement
		 */
		public function execute(){
			if(!$this->connection){
				fatal_error(DATABASE_EXECUTE_ERROR, "There was an attempt to execute a query using a closed connection.");
				return;
			}
			
			$arguments = func_get_args();
			$statement = $arguments[0];
			$expectedParameters = substr_count($statement, "?");
			$parameters = array_slice($arguments, 1, $expectedParameters);
			$parameterReferences = array();
		
			//Convert all $parameters into references
			for($i = 0; $i < sizeof($parameters); $i++){
				if(!is_null($parameters[$i]) && !is_scalar($parameters[$i])){
					fatal_error(DATABASE_EXECUTE_ERROR, "Query parameter of type ".gettype($parameters[$i])." was passed to".
										" MySQLConnection::execute(). Primitive type expected.");
					return;					
				}				
				
				$parameterReferences[] = &$parameters[$i];
			}
			
			$preparedStatement = $this->connection->prepare($statement);
			
			if(!$preparedStatement){
				fatal_error(DATABASE_EXECUTE_ERROR, $this->connection->error.". $statement");
				return;
			}
			
			$bindString = "";
			foreach($parameters as $parameter){
				if(is_int($parameter) || is_bool($parameter) || is_null($parameter)){
					$bindString .= "i";
				}else if(is_float($parameter) || is_double($parameter)){
					$bindString .= "d";
				}else{
					$bindString .= "s";
				}
			}
			
			if($bindString){
				$bindParameters = array_merge(array($bindString), $parameters);
				@call_user_func_array(array($preparedStatement, "bind_param"), $bindParameters);
			}
			
			if(@!$preparedStatement->execute()){
				echo $statement;
				fatal_error(DATABASE_EXECUTE_ERROR, $preparedStatement->error.".");
				return;
			}
			
			$bindVariables = array();
			$resultSet = array();
			$resultRow = array();
			
			//If a non query statement was executed the metadata should be null and we return true for success.
			if(!$preparedStatement->result_metadata()){
				return $resultSet;
			}
			
			$resultFields = $preparedStatement->result_metadata()->fetch_fields();
			
			foreach($resultFields as $resultField){
				$bindVariables[] = &$resultRow[$resultField->name];
			}
			
			call_user_func_array(array($preparedStatement, 'bind_result'), $bindVariables);			
			
			while($preparedStatement->fetch()){
				$nextRow = array();
				foreach($resultFields as $resultField){
					$nextRow[$resultField->name] = $resultRow[$resultField->name];
				}
				
				$resultSet[] = $nextRow;
			}
			
			$preparedStatement->close();
			return $resultSet;
		}
		
		/*
		 * Commits the current transaction
		 */
		public function commit(){
			if($this->connection){
				$this->connection->commit();
			}
		}
		
		/*
		 * Rollbacks the current transaction
		 */
		public function rollback(){
			if($this->connection){
				$this->connection->rollback();
			}
		}
		
		/*
		 * Closes a connection to a MySQL database server.
		 */
		public function close(){
			if($this->connection){
				$this->connection->close();
			}
			
			$this->connection = null;
		}
		
		/*
		 * Checks if the connection is open.
		 */
		public function isClosed(){
			return is_null($this->connection);
		}
	}
?>