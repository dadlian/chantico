<?php
	abstract class UserController extends ChanticoController{
		public function execute(){			
			if(RequestHandler::getAuthenticatedToken()->getRole() == "user"){
				$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
				$authorisation = RequestHandler::getAuthorisation();
				
				if($user && !($user->getUsername() == strtolower($authorisation["key"]) && $user->getAuthentication() == md5($authorisation["secret"]))){
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
			
			$usernames = RequestHandler::getQueryParameter("usernames");
			if(!is_string($usernames)){
				$invalidQueryParameters[] = "usernames";
			}
			
			$options = RequestHandler::getQueryParameter("options");
			if($options && !preg_match("/^\w+~[\w:\/\-\.]+(,\w+~[\w:\/\-\.]+)*$/",$options)){
				$invalidQueryParameters[] = "options";
			}
			
			$access = RequestHandler::getQueryParameter("access");
			if(!is_string($access)){
				$invalidQueryParameters[] = "access";
			}
			
			return $invalidQueryParameters;
		}
		
		protected function getRecordCount(){
			$count = 0;
			if(DatabaseAdministrator::tableExists("ManagedAccess")){
				$usernames = RequestHandler::getQueryParameter("usernames");
				$options = RequestHandler::getQueryParameter("options");
				$access = RequestHandler::getQueryParameter("access");
				
				$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
				$apiCheck = "api = {$api->getId()}";
				if($usernames){
					$patterns = array();
					foreach(explode(",",$usernames) as $username){
						$username = strtolower($username);
						$patterns[] = "username LIKE '$username'";
					}
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess WHERE $apiCheck AND (".implode(" OR ",$patterns).")");
				}else if($options){
					$optionsList = explode(",",$options);
					$optionsQueryParts = array();
					foreach($optionsList as $option){
						$parameters = preg_split("/~/",$option);
						$optionsQueryParts[] = "(name = '{$parameters[0]}' AND value = '{$parameters[1]}')";
					}
					
					$optionsQuery = implode(" OR ",$optionsQueryParts);
					$query = "SELECT COUNT(A.id) as user FROM ManagedAccess as A JOIN ManagedAccessOptions as B ON A.id = managedaccess WHERE ($optionsQuery) AND A.$apiCheck";
					$countResult = DatabaseAdministrator::execute($query);
				}else if($access){
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess WHERE accessEndpoint = '$access' AND $apiCheck");
				}else{
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess WHERE $apiCheck");
				}
				
				$count = $countResult[0]["user"];
			}
			
			return $count;
		}
		
		protected function buildPageEntries($start,$count){
			$payload = array();
				
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$usernames = RequestHandler::getQueryParameter("usernames");
			$options = RequestHandler::getQueryParameter("options");
			$access = RequestHandler::getQueryParameter("access");
			
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$apiCheck = "api = {$api->getId()}";
			if($usernames){
				$patterns = array();
				foreach(explode(",",$usernames) as $username){
					$username = strtolower($username);
					$patterns[] = "username LIKE '$username'";
				}
				
				$results = DatabaseAdministrator::execute("SELECT id FROM ManagedAccess WHERE $apiCheck AND (".implode(" OR ",$patterns).")");
				$users = array();
				foreach($results as $user){
					$users[] = $user["id"];
				}
				
				$searcher->addCriterion("id",Criterion::EQUAL,$users);
			}else if($options){
				$optionsList = explode(",",$options);
				$optionsQueryParts = array();
				foreach($optionsList as $option){
					$parameters = preg_split("/~/",$option);
					$optionsQueryParts[] = "(name = '{$parameters[0]}' AND value = '{$parameters[1]}')";
				}
				
				$optionsQuery = implode(" OR ",$optionsQueryParts);
				$results = DatabaseAdministrator::execute("SELECT A.id as id FROM ManagedAccess as A JOIN ManagedAccessOptions as B ON A.id = managedaccess WHERE ($optionsQuery) AND A.$apiCheck");
				
				$users = array();
				foreach($results as $user){
					$users[] = $user["id"];
				}
				
				$searcher->addCriterion("id",Criterion::EQUAL,$users);
			}else if($access){
				$searcher->addCriterion("accessEndpoint",Criterion::EQUAL,$access);
			}
			
			$searcher->addCriterion("api",Criterion::EQUAL,$api->getId());
			foreach($sqlGateway->find("ManagedAccess",$searcher,new Sorter(),$count,$start) as $user){
				$payload[] = $this->assemblePayload($user);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($user){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));

			//Load User Options from API
			$realOptions = array();
			foreach($user->getOptions() as $option => $value){
				if($option != "role"){
					$realOptions[$option] = $value;
				}
			}
			
			$optionsInitialised = sizeof($realOptions) > 0;
			foreach($realOptions as $option){
				if(!$option){
					$optionsInitialised = false;
					break;
				}
			}
			
			if(!$optionsInitialised){
				$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
				$request->setTransport("ssl");
				$request->setPort(443);
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setEndpoint($user->getAccessEndpoint());
				$response = $request->get();
			
				$options = array();
				if($response->getStatusCode() == "200"){
					$responseBody = json_decode($response->getBody(),true);
					foreach($responseBody as $option => $value){
						if(!in_array($option,array("self","tokens","active-token"))){
							$options[$option] = $value;
						}
					}
				}
				
				$user->setOptions($options);
				$sqlGateway = new SQLGateway();
				$sqlGateway->save($user);
			}
			
			$options = $user->getOptions();
			unset($options["role"]);
			
			$userURI = preg_replace("/\{api_id\}/",$api->getId(),$user->getURI());
			$payload = array(
				"self"=>$userURI,
				"username"=>$user->getUsername(),
				"role"=>$user->viewFromOptions("role"),
				"tokens"=>"$userURI/tokens",
				"options"=>$options,
				"resets"=>"$userURI/resets",
				"recoveries"=>"$userURI/recoveries"
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