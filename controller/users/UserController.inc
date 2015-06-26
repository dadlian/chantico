<?php
	abstract class UserController extends ChanticoController{
		public function execute(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			
			if(RequestHandler::getAuthenticatedToken()->getRole() == "user"){
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
			
			$usernames = RequestHandler::getQueryParameter("usernames");
			if(!is_string($usernames)){
				$invalidQueryParameters[] = "usernames";
			}
			
			$options = RequestHandler::getQueryParameter("options");
			if($options && !preg_match("/^\w+~[\w:\/\-\.]+$/",$options)){
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
				$apiCheck = "id IN (SELECT value FROM APIUsers WHERE api = '{$api->getId()}')";
				if($usernames){
					$patterns = array();
					foreach(explode(",",$usernames) as $username){
						$patterns[] = "username LIKE '$username'";
					}
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess WHERE $apiCheck AND (".implode(" OR ",$patterns).")");
				}else if($options){
					$parameters = preg_split("/~/",$options);
					$query = "SELECT COUNT(A.id) as user FROM ManagedAccess as A JOIN ManagedAccessOptions as B ON A.id = managedaccess ".
							"WHERE name = '{$parameters[0]}' AND value = '{$parameters[1]}' AND A.$apiCheck";
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
			$apiCheck = "id IN (SELECT value FROM APIUsers WHERE api = '{$api->getId()}')";
			if($usernames){
				$patterns = array();
				foreach(explode(",",$usernames) as $username){
					$patterns[] = "username LIKE '$username'";
				}
				
				$results = DatabaseAdministrator::execute("SELECT id FROM ManagedAccess WHERE $apiCheck AND (".implode(" OR ",$patterns).")");
				$users = array();
				foreach($results as $user){
					$users[] = $user["id"];
				}
				
				$searcher->addCriterion("id",Criterion::EQUAL,$users);
			}else if($options){
				$parameters = preg_split("/~/",$options);
				$results = DatabaseAdministrator::execute("SELECT A.id FROM ManagedAccess as A JOIN ManagedAccessOptions as B ON A.id = managedaccess ".
										"WHERE A.$apiCheck AND name = '{$parameters[0]}' AND value = '{$parameters[1]}'");
				$users = array();
				foreach($results as $user){
					$users[] = $user["id"];
				}
				
				$searcher->addCriterion("id",Criterion::EQUAL,$users);
			}else if($access){
				$searcher->addCriterion("accessEndpoint",Criterion::EQUAL,$access);
			}
			
			$searcher->addCriterion("API",$api->getId(),"users");
			foreach($sqlGateway->find("ManagedAccess",$searcher,new Sorter(),$count,$start) as $user){
				$payload[] = $this->assemblePayload($user);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($user){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			$request->authorise($api->getAccessKey(),$api->getAccessSecret());
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
			
			$userURI = preg_replace("/\{api_id\}/",$api->getId(),$user->getURI());
			$payload = array(
				"self"=>$userURI,
				"username"=>$user->getUsername(),
				"role"=>$role,
				"tokens"=>"$userURI/tokens",
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