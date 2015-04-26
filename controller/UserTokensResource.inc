<?php
	class UserTokensResource extends UserController{
		protected function isInvalid(){
			return array();
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
		
		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			return $invalidQueryParameters;
		}
		
		protected function get(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			
			$authentication = RequestHandler::getAuthorisation();
			$username = $authentication["key"];
			$password = $authentication["secret"];
			
			//Retrieve user access resource
			$request = new WadapiRequest(parse_url(SettingsManager::getSetting("options","apiuri"),PHP_URL_HOST));
			$request->setEndpoint($user->getAccessEndpoint());
			$request->authorise(SettingsManager::getSetting("options","key"),SettingsManager::getSetting("options","secret"));
			$request->setContentType("application/json");
			$response = $request->get();
			
			if($response->getStatusCode() != 200){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}
			
			$access = json_decode($response->getBody(),true);
			
			if($user->getExpires() <= time()){
				//Refresh the user token
				$accessEndpoint = parse_url($access["tokens"],PHP_URL_PATH);
				$request->setEndpoint($accessEndpoint);
				$request->authorise($this->decrypt($user->getAccessKey(),$password),$this->decrypt($user->getRefreshSecret(),$password));
				$request->setBody(json_encode(array("lifetime"=>SettingsManager::getSetting("options","lifetime"))));
				$response = $request->post();
				if($response->getStatusCode() == 201){
					$body = json_decode($response->getBody(),true);
					$accessKey = $body["key"];
					$accessSecret = $body["secret"];
					$refreshSecret = $body["refresh"];
					$lifetime = $body["lifetime"];
					
					//Save User to Chantico
					$user->setAccessKey($this->encrypt($accessKey,$password));
					$user->setAccessSecret($this->encrypt($accessSecret,$password));
					$user->setRefreshSecret($this->encrypt($refreshSecret,$password));
					$user->setExpires(time()+$lifetime);
					
					$sqlGateway = new SQLGateway();
					$sqlGateway->save($user);
				}
			}
			
			$payload = array(
				"self"=>$this->getBase()."/".RequestHandler::getRequestURI(),
				"key"=>$this->decrypt($user->getAccessKey(),$password),
				"secret"=>$this->decrypt($user->getAccessSecret(),$password),
				"expiry"=>$user->getExpires(),
				"renew"=>$user->getURI()."/renewals?warrant=".Magistrate::issueWarrant("renew")
			);
			ResponseHandler::retrieved($payload,$this->getBase()."/".RequestHandler::getRequestURI(),$user->getModified(),$user->getETag());
		}
	}
?>