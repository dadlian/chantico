<?php
	class DatabaseAdministrator extends Worker{
		/*
		 * The active database connection
		 */
		private static $activeConnection;
		
		/*
		 * System table prefix loaded once from configuration
		 */
		private static $tablePrefix = null;
		
		/*
		 * System index hits loaded once from configuration
		 */
		private static $indexHits = null;
		
		/*
 		 * Stores the tables known to exist so they don't have to be looked up more than once
 		 */
 		private static $knownTables = array();
		
		
		/*
		 * Sends the provided statement to the connected server
		 */
		public static function execute(){
			$arguments = func_get_args();
			$statement = $arguments[0];
			
			if(!self::checkConnection()){
				return;
			}
			
			$expectedParameterCount = substr_count($statement, "?");
			$statementParameters = array_slice($arguments, 1, $expectedParameterCount);
			$statementParameterCount = sizeof($statementParameters);
			
			if($statementParameterCount < $expectedParameterCount){
				fatal_error(DATABASE_EXECUTE_ERROR, "Incorrect number of parameters passed to DatabaseAdministrator::execute(). ".
						"$statementParameterCount Passed, $expectedParameterCount Expected.");
				return;
			}
			
			$statement = self::addPrefixes($statement);
			$sqlStatementString = new SQLString($statement);
			
			//If this is a select query, manage indexes
			/*if($sqlStatementString->getType() == SQLString::SELECT_STRING){
				self::manageIndexes($sqlStatementString);
			}*/
			
			//Execute parameterised statement
			$executeParameters = array_merge(array($statement), $statementParameters);			
			$result = call_user_func_array(array(self::$activeConnection, "execute"), $executeParameters);
			
			if($sqlStatementString->getType() == SQLString::CREATE_STRING){
				self::$knownTables = array_merge(array_keys($sqlStatementString->getTables()),self::$knownTables);
			}else if($sqlStatementString->getType() == SQLString::DROP_STRING){
				self::$knownTables = array_diff(self::$knownTables,array_keys($sqlStatementString->getTables()));
			}else if($sqlStatementString->getType() == SQLString::RENAME_STRING){
				$tables = array_keys($sqlStatementString->getTables());
				$from = $tables[0]; $to = $tables[1];
				self::$knownTables = array_merge(array_diff(self::$knownTables, array($from)), array($to));
			}
			
			return $result;
		}
		
		/*
		 * Returns true if a table with the given name already exists. False otherwise.
		 */
		public static function tableExists($tableName){	
			$tablePrefix = self::getTablePrefix();
			
			if(!self::$knownTables){
				$results = self::execute("SHOW TABLES");	
				foreach($results as $result){
					self::$knownTables[] = array_shift($result);
				}
			}
			
			$prefix = self::getTablePrefix();
			return in_array("$prefix$tableName", self::$knownTables);
		}
		
		/*
		 * Returns the ID of the last database insert
		 */
		public static function getLastInsertId(){
			$insertIdResultSet = self::execute("SELECT LAST_INSERT_ID() as insert_id");
			return $insertIdResultSet[0]['insert_id'];
		}
		
		/*
		 * Commits active transaction to DB
		 */
		public static function commit(){
			if(self::$activeConnection){
				self::$activeConnection->commit();
			}
		}
		
		/*
		 * Rollsback active transaction to DB
		 */
		public static function rollback(){
			if(self::$activeConnection){
				self::$activeConnection->rollback();
			}
		}
		
		//Builds a connection using the configured connection parameters
		protected static function connect(){
			self::buildConnection(SettingsManager::getSetting('Database','Hostname'),SettingsManager::getSetting('Database','Username'),
						SettingsManager::getSetting('Database','Password'),SettingsManager::getSetting('Database','Database'));
		}
		
		//Builds the connection using the passed connection parameters
		protected static function buildConnection($hostname, $username, $password, $database){
			if(self::$activeConnection){
				self::$activeConnection->close();
			}

			$databaseConnection = new MySQLConnection($hostname, $username, $password, $database);
			$databaseConnection->connect();
			self::$activeConnection = $databaseConnection;
			self::$knownTables = array();
		}
		
		/*
		 * Tells the administrator to use the system wide DB connection if none has been specified
		 */
		private static function checkConnection(){
			if(!self::$activeConnection){
				self::connect();
			}
			
			return !self::$activeConnection->isClosed();
		}
		
		/*
		 * Adds the system specifed table prefix, before all table names in statement
		 */
		private static function addPrefixes($statement){
			$sqlStatementString = new SQLString($statement);
			$statementTables = $sqlStatementString->getTables();
			
			//Add table prefix only if one exists
			$tablePrefix = self::getTablePrefix();
			if($tablePrefix){				
				foreach($statementTables as $table => $alias){
					$statement = preg_replace("/([,\s])$table([,\s;\)\(]|$)/", "$1$tablePrefix$table$2", $statement);
				}
			}
					
			return $statement;
		}
		
		/*
		 * Return the table prefix to inset into queries
		 */
		private static function getTablePrefix(){
			if(is_null(self::$tablePrefix)){			
				if(SettingsManager::getSetting('Database','Table Prefix')){
					self::$tablePrefix = SettingsManager::getSetting('Database','Table Prefix')."_";
				}else{
					self::$tablePrefix = "";
				}
			}
			
			return self::$tablePrefix;
		}
		
		/*
		 * Return the index hits value
		 */
		private static function getHitsForIndex(){
			if(is_null(self::$indexHits)){
				if(SettingsManager::getSetting('Database','Table Prefix')){
					self::$indexHits = SettingsManager::getSetting('Database','Hits For Index');
				}else{
					self::$indexHits = "";
				}
			}
			
			return self::$indexHits;
		}
		
		/*
		 * Given a SELECT statement, add indexes on heavily referenced columns
		 */
		private static function manageIndexes($sqlStatementString){
			//Create ColumnHits table if not already created
			if(!self::tableExists("ColumnHits")){
				$createColumnHitQuery = self::addPrefixes("CREATE TABLE ColumnHits(
								tableName VARCHAR(256) NOT NULL,
								columnName VARCHAR(256) NOT NULL,
								hits INT NOT NULL DEFAULT 0,
								indexed BOOL NOT NULL DEFAULT 0,
								PRIMARY KEY (tableName, columnName)
							)ENGINE=INNODB;");			
				self::$activeConnection->execute($createColumnHitQuery);
			}
			
			//Find Statement table(s) and column(s)
			$statementTables = $sqlStatementString->getTables();
			$statementColumns = $sqlStatementString->getColumns();
					
			//Get list of columns for each table in the statement
			$tableColumns = array();
			$keyColumns = array();
			
			foreach($statementTables as $table => $alias){
				$tableColumns[$table] = array();
				$columnRows = self::$activeConnection->execute("SHOW COLUMNS FROM $table");
				foreach($columnRows as $columnRow){
					$tableColumns[$table][] = $columnRow['Field'];
					if($columnRow['Key'] == "PRI" || $columnRow['Key'] == "MUL" || $columnRow['Type'] == "text"){
						$keyColumns[] = $columnRow['Field'];
					}
				}
			}
			
			//Get hits for the particular column/table
			$hitSelectQuery = self::addPrefixes("SELECT hits,indexed,tableName,columnName FROM ColumnHits");
			$hitsResult = array();
			foreach(self::$activeConnection->execute($hitSelectQuery) as $result){
				$hitsResult["{$result['tableName']}:{$result['columnName']}"] = $result['hits'];
			}
			
			//Find out which table a column belongs to and update the ColumnHits table
			$columnTable = "";			
			foreach($statementColumns as $column => $columnTableAlias){
				$columnPieces = preg_split("/\./", $column);
				$column = $columnPieces[sizeof($columnPieces)-1];
				
				foreach($statementTables as $table => $alias){
					if(($alias && $columnTableAlias == $alias) || (!$alias && in_array($column, $tableColumns[$table]))){
						$columnTable = $table;
					}
				}
				
				$newEntry = true;
				$hits = 0;
				
				if(in_array($column, $keyColumns)){
					$indexed = true;
				}else{
					$indexed = false;
				}
				
				if(array_key_exists("$columnTable:$column",$hitsResult)){
					$newEntry = false;
					$hits = $hitsResult["$columnTable:$column"];
				}
				
				//Check if the new hits total requires us to add an index
				$hits+=1;
				
				if(!$indexed && $hits > self::getHitsForIndex()){
					$indexed = true;
					$indexQuery = "CREATE INDEX {$column}_index ON $columnTable ($column)";
					self::$activeConnection->execute($indexQuery);
				}
				
				//Update the hit count and indexed status in the ColumnHits table
				if($newEntry){
					$hitUpdateQuery = self::addPrefixes("INSERT INTO ColumnHits VALUES(?,?,?,?)");
					self::$activeConnection->execute($hitUpdateQuery, "$columnTable", $column, $hits, $indexed);
				}else{
					$hitUpdateQuery = self::addPrefixes("UPDATE ColumnHits SET hits=?, indexed=? WHERE tableName=? AND columnName=?");
					$preparedStatement = self::$activeConnection->execute($hitUpdateQuery, $hits, $indexed, "$columnTable", $column);
				}
			}
		}
	}
?>