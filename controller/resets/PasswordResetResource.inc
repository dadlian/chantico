<?php
	class PasswordResetResource extends PasswordResetController{
		protected function get(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$reset = $this->getResourceObject("PasswordReset","id",$this->viewFromArguments("reset"));
			
			$payload = $this->assemblePayload($reset);
			ResponseHandler::retrieved($payload,$reset->getURI(),$reset->getModified(),$reset->getETag());
		}
		
		protected function put(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$reset = $this->getResourceObject("PasswordReset","id",$this->viewFromArguments("reset"));
			
			if(in_array($reset->getStatus(),array("reset","cancelled"))){
				ResponseHandler::conflict("The password reset request has been resolved, and can no longer be modified.");
			}
			
			if($reset->getCode() !== $this->getFromContent("code")){
				ResponseHandler::forbidden("The given password reset code is invalid for this request.");
			}
			
			$sqlGateway = new SQLGateway();
			
			//Get user's APIToken
			$searcher = new Searcher();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
			$apiToken = $sqlGateway->findUnique("APIToken",$searcher);
			
			//Save user changes to Chantico
			$authentication = $this->getFromContent("authentication");
			
			//Find User Role
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			if($api->isSecure()){
				$request->setTransport("ssl");
				$request->setPort(443);
			}
			
			$request->setEndpoint($user->getAccessEndpoint());
			$request->authorise($api->getAccessKey(),$api->getAccessSecret());
			$request->setContentType("application/json");
			$response = $request->get();
			$responseBody = json_decode($response->getBody(),true);
			$role = $responseBody["role"];
			$options = array();
			foreach($responseBody as $option => $value){
				if(!in_array($option,array("self","tokens","active-token","role"))){
					$options[$option] = $value;
				}
			}
			
			//Create Fresh Access Tokens
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			if($api->isSecure()){
				$request->setTransport("ssl");
				$request->setPort(443);
			}
			
			$request->setEndpoint(parse_url($api->getRoot(),PHP_URL_PATH)."/access");
			$request->authorise($api->getAccessKey(),$api->getAccessSecret());
			$request->setContentType("application/json");
			$payload = json_encode(array_merge(array("role"=>$role),$options));
			$request->setBody($payload);
			$response = $request->post();
			$body = json_decode($response->getBody(),true);
			
			if($response->getStatusCode() == 400){
				ResponseHandler::bad(str_replace("arguments","directives",$body["message"]));
			}else if($response->getStatusCode() != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}
			
			$accessEndpoint = parse_url($body["self"],PHP_URL_PATH);
			$tokensEndpoint = parse_url($body["tokens"],PHP_URL_PATH);
			$accessKey = $body["active-token"]["key"];
			$refreshSecret = $body["active-token"]["refresh"];
			
			//Refresh Access Token
			$request->setEndpoint($tokensEndpoint);
			$request->authorise($accessKey,$refreshSecret);
			$request->setBody(json_encode(array("lifetime"=>SettingsManager::getSetting("options","lifetime"))));
			$response = $request->post();
			
			if($response->getStatusCode() != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}
			
			//Encrypt and save new access tokens
			$body = json_decode($response->getBody(),true);
			$accessKey = $body["key"];
			$accessSecret = $body["secret"];
			$refreshSecret = $body["refresh"];
			$lifetime = $body["lifetime"];
			
			$user->setAuthentication(md5($authentication));
			$user->setAccessKey($this->encrypt($accessKey,$authentication));
			$user->setAccessSecret($this->encrypt($accessSecret,$authentication));
			$user->setRefreshSecret($this->encrypt($refreshSecret,$authentication));
			$user->setAccessEndpoint($accessEndpoint);
			$user->setExpires(time()+$lifetime);
			
			$user->setOptions($user->getOptions());
			$sqlGateway->save($user);
			
			//Update APIToken for User to Access its
			$apiToken->setAccessKey(md5($user->getUsername()));
			$apiToken->setAccessSecret($user->getAuthentication());
			$apiToken->setRefreshSecret($user->getAuthentication());
			$sqlGateway->save($apiToken);
			
			//Update Password Reset Status
			$reset->setStatus("reset");
			$sqlGateway->save($reset);
			
			$payload = $this->assemblePayload($reset);
			ResponseHandler::modified($payload,$reset->getURI(),$reset->getModified(),$reset->getETag());
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			$reset = $this->getResourceObject("PasswordReset","id",$this->viewFromArguments("reset"));
			return $modifiedDate == $reset->getModified() && $eTag == $reset->getETag();
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
	}
?>