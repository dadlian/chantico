<?php
	abstract class UserController extends PagedResourceController{
		public function execute(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			
			$authorisation = RequestHandler::getAuthorisation();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($authorisation["key"]));
			$token = $sqlGateway->findUnique("APIToken",$searcher);
			if($token->getRole() == "user"){
				$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
				$authorisation = RequestHandler::getAuthorisation();
				
				if($user && !($user->getUsername() == $authorisation["key"] && $user->getAuthentication() == md5($authorisation["secret"]))){
					ResponseHandler::forbidden("The provided tokens do not have permission to perform this action.");
				}
			}
			
			parent::execute();
		}
		
		protected function isInvalid(){
			$invalidArguments = array();
			
			if(!is_string($this->getFromContent("username")) || preg_match("/\s/",$this->getFromContent("username"))){
				$invalidArguments[] = "username";
			}
			
			if(!is_string($this->getFromContent("authentication"))){
				$invalidArguments[] = "authentication";
			}
			
			if($this->getFromContent("role") && !is_string($this->getFromContent("role"))){
				$invalidArguments[] = "role";
			}
			
			if(!is_array($this->getFromContent("options",array()))){
				$invalidArguments[] = "options";
			}else{
				foreach($this->getFromContent("options",array()) as $option){
					if(!is_string($option)){
						$invalidArguments[] = "options";
						break;
					}
				}
			}
			
			return $invalidArguments;
		}
		
		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			
			$username = RequestHandler::getQueryParameter("username");
			if(!is_string($username)){
				$invalidQueryParameters[] = "username";
				break;
			}
			
			$options = RequestHandler::getQueryParameter("options");
			if($options && !preg_match("/^\w+~[\w:\/\-\.]+$/",$options)){
				$invalidQueryParameters[] = "options";
			}
			
			return $invalidQueryParameters;
		}
		
		protected function getRecordCount(){
			$count = 0;
			if(DatabaseAdministrator::tableExists("ManagedAccess")){
				$username = RequestHandler::getQueryParameter("username");
				$options = RequestHandler::getQueryParameter("options");
				if($username){
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess WHERE username LIKE '%$username%'");
				}else if($options){
					$parameters = preg_split("/~/",$options);
					$query = "SELECT COUNT(A.id) as user FROM ManagedAccess as A JOIN ManagedAccessOptions as B ON A.id = managedaccess ".
							"WHERE name = '{$parameters[0]}' AND value = '{$parameters[1]}'";
					$countResult = DatabaseAdministrator::execute($query);
				}else{
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess");
				}
				
				$count = $countResult[0]["user"];
			}
			
			return $count;
		}
		
		protected function buildPageEntries($start,$count){
			$payload = array();
				
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$username = RequestHandler::getQueryParameter("username");
			$options = RequestHandler::getQueryParameter("options");
			if($username){
				$searcher->addCriterion("username",Criterion::LIKE,"%$username%");
			}else if($options){
				$parameters = preg_split("/~/",$options);
				$results = DatabaseAdministrator::execute("SELECT A.id FROM ManagedAccess as A JOIN ManagedAccessOptions as B ON A.id = managedaccess ".
								"WHERE name = '{$parameters[0]}' AND value = '{$parameters[1]}'");
				$users = array();
				foreach($results as $user){
					$users[] = $user["id"];
				}
				
				$searcher->addCriterion("id",Criterion::EQUAL,$users);
			}
			
			foreach($sqlGateway->find("ManagedAccess",$searcher,new Sorter(),$count,$start) as $user){
				$payload[] = $this->assemblePayload($user);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($user){
			$request = new WadapiRequest(parse_url(SettingsManager::getSetting("options","apiuri"),PHP_URL_HOST));
			$request->authorise(SettingsManager::getSetting("options","key"),SettingsManager::getSetting("options","secret"));
			$request->setEndpoint($user->getAccessEndpoint());
			$response = $request->get();
			
			if($response->getStatusCode() == "200"){
				$responseBody = json_decode($response->getBody(),true);
				$role = $responseBody["role"];
				$options = array();
				foreach($responseBody as $option => $value){
					if(!in_array($option,array("self","tokens","active-token","role"))){
						$options[$option] = $value;
					}
				}
			}else{
				return array();
			}
		
			$payload = array(
				"self"=>$user->getURI(),
				"username"=>$user->getUsername(),
				"role"=>$role,
				"tokens"=>"{$user->getURI()}/tokens",
				"options"=>$options
			);
			
			return $payload;
		}
		
		protected function encrypt($data,$key){
			$key = md5(password_hash($key,PASSWORD_DEFAULT,array("salt"=>SettingsManager::getSetting("options","salt"))));
			$key = pack('H*',$key);
			
			# create a random IV to use with CBC encoding
			$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CBC);
			$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
			
			# creates a cipher text compatible with AES (Rijndael block size = 128)
			$ciphertext = $iv . mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $key, $data, MCRYPT_MODE_CBC, $iv);
			return base64_encode($ciphertext);
		}
		
		protected function decrypt($data,$key){
			$key = md5(password_hash($key,PASSWORD_DEFAULT,array("salt"=>SettingsManager::getSetting("options","salt"))));
			$key = pack('H*',$key);
			$ciphertext = base64_decode($data);
			
			# retrieves the IV, iv_size should be created using mcrypt_get_iv_size()
			$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CBC);
			$iv = substr($ciphertext, 0, $iv_size);
			
			# retrieves the cipher text (everything except the $iv_size in the front)
			$ciphertext = substr($ciphertext, $iv_size);
			return mcrypt_decrypt(MCRYPT_RIJNDAEL_128, $key, $ciphertext, MCRYPT_MODE_CBC, $iv);
		}
	}
?>