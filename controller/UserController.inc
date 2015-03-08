<?php
	abstract class UserController extends PagedResourceController{
		public function execute(){
			if($this->viewFromArguments("user")){
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
				break;
			}
			
			return $invalidQueryParameters;
		}
		
		protected function getRecordCount(){
			$count = 0;
			if(DatabaseAdministrator::tableExists("ManagedAccess")){
				$usernames = RequestHandler::getQueryParameter("usernames");
				$parameters = array();
				if($usernames){
					foreach(explode(",",$usernames) as $username){
						$parameters[] = "?";
					}
					
					$query = "SELECT COUNT(id) as user FROM ManagedAccess WHERE username IN (".implode(",",$parameters).")";
					$countResult = call_user_func_array(array("DatabaseAdministrator","execute"),array_merge(array($query),explode(",",$usernames)));
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
			$usernames = RequestHandler::getQueryParameter("usernames");
			$parameters = array();
			if($usernames){
				$searcher->addCriterion("username",Criterion::EQUAL,explode(",",$usernames));
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