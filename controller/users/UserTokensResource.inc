<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\System\SettingsManager;

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
			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");

			$authentication = RequestHandler::getAuthorisation();
			$username = $authentication["key"];
			$password = $authentication["secret"];

			//Retrieve user access resource
			$response = $this->sendInstanceGet($user->getAccessEndpoint());
			if($response['code'] != 200){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			$access = $response['body'];
			if(!$user->isMultiSession() && $user->getExpires() <= time()){
				//Refresh the user token
				$tokensEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$access["tokens"]);
				$accessKey = $this->decrypt($user->getAccessKey(),$password);
				$refreshSecret = $this->decrypt($user->getAccessKey(),$password);
				$payload = array('invalidated-token' => $instanceUrl.$user->getAccessEndpoint());
				$response = $this->sendInstancePost($tokensEndpoint,$payload,$accessKey,$refreshSecret);

				if($response['code'] == 201){
					$accessKey = $response["body"]["key"];
					$accessSecret = $response["body"]["secret"];
					$refreshSecret = $response["body"]["refresh"];
					$lifetime = $response["body"]["lifetime"];

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
				"expiry"=>$user->getExpires()?($user->getExpires()-time()):0
			);

			ResponseHandler::retrieved($payload,$this->getBase()."/".RequestHandler::getRequestURI(),$user->getModified(),$user->getETag());
		}
	}
?>
