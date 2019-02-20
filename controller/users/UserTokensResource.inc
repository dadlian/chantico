<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;

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
			$response = $this->sendInstanceGet($user->getAccessEndpoint());
			if($response['code'] != 200){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			$access = $response['body'];
			if(!$user->isMultiSession() && $user->getExpires() <= time()){
				//Refresh the user token
				$accessEndpoint = parse_url($access["tokens"],PHP_URL_PATH);
				$accessKey = $this->decrypt($user->getAccessKey(),$password);
				$refreshSecret = $this->decrypt($user->getAccessKey(),$password);
				$response = $this->sendInstancePost($accessEndpoint,array(),$accessKey,$refreshSecret);
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

			//We can re-enable token renewals if it ever becomes necessary. For now they have infinite lifespans, but
			//are invalidated each time they are requested.
			//Generate a new renewal stub
			/*$warrant = Magistrate::issueWarrant("renew");

			$stubToken = $warrant.$_SERVER['REMOTE_ADDR'];
			$accessKey = $this->encrypt($this->decrypt($user->getAccessKey(),$password),$stubToken);
			$refreshSecret = $this->encrypt($this->decrypt($user->getRefreshSecret(),$password),$stubToken);
			$userPassword = $this->encrypt($password,$stubToken);
			$stub = new RenewalStub(md5($stubToken),$accessKey,$refreshSecret,$userPassword);
			$sqlGateway->save($stub);*/

			$payload = array(
				"self"=>$this->getBase()."/".RequestHandler::getRequestURI(),
				"key"=>$this->decrypt($user->getAccessKey(),$password),
				"secret"=>$this->decrypt($user->getAccessSecret(),$password),
				"expiry"=>$user->getExpires()-time()
			);

			ResponseHandler::retrieved($payload,$this->getBase()."/".RequestHandler::getRequestURI(),$user->getModified(),$user->getETag());
		}
	}
?>
