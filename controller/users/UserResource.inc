<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\System\SettingsManager;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;
	use Wadapi\Persistence\CryptKeeper;
	use Wadapi\Persistence\Photographer;

	class UserResource extends UserController{
		protected function get(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$payload = $this->assemblePayload($user);
			ResponseHandler::retrieved($payload,$user->getURI(),$user->getModified(),$user->getETag());
		}

		protected function put(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$normalisedUsername = strtolower($this->getFromContent("username"));
			$sqlGateway = new SQLGateway();

			//Set new user values
			$authentication = $this->getFromContent("authentication");
			$authentication = $authentication?md5($authentication):$user->getAuthentication();

			//Check whether username already exists
			$user->setUsername($normalisedUsername);
			if($user->isDirty('username')){
				$searcher = new Searcher();
				$searcher->addCriterion("username",Criterion::EQUAL,$normalisedUsername);
				$searcher->addCriterion("id",Criterion::NOT_EQUAL,$user->getId());
				if($sqlGateway->findUnique("ManagedAccess",$searcher)){
					ResponseHandler::conflict("A user identified by '{$this->getFromContent('username')}' already exists.");
				}
			}

			//Update User Options
			$userOptions = array();
			foreach($user->getOptions() as $option => $value){
				$userOptions[$option] = json_decode($value);
			}

			$user->insertToOptions("role",$this->getFromContent("role"));
			$userOptions["role"] = $this->getFromContent("role");

			if($user->isDirty("options")){
				$this->sendInstancePut($user->getAccessEndpoint(),$userOptions);
			}

			//Update user password if necessary
			$user->setAuthentication($authentication);
			if($user->isDirty("authentication")){
				//Create Fresh Access Tokens
				$payload = $userOptions;
				$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");

				$payload['invalidated-token'] = $instanceUrl.$user->getAccessEndpoint();
				$response = $this->sendInstancePost("access",$payload);

				if($response['code'] == 400){
					ResponseHandler::bad(str_replace("arguments","directives",$response["body"]["message"]));
				}else if($response['code'] != 201){
					ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
				}

				//Delete old token
				$this->sendInstanceDelete($user->getAccessEndpoint());

				foreach($response['body']['profile'] as $option => $value){
					$user->insertToOptions($option,json_encode($value));
				}

				$accessEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$response["body"]["self"]);
				$tokensEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$response["body"]["tokens"]);
				$accessKey = $response['body']["active-token"]["key"];
				$refreshSecret = $response['body']["active-token"]["refresh"];

				//Refresh Access Token
				$response = $this->sendInstancePost($tokensEndpoint,array(),$accessKey,$refreshSecret);

				if($response['code'] != 201){
					ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
				}

				$accessKey = $response['body']["key"];
				$accessSecret = $response['body']["secret"];
				$refreshSecret = $response['body']["refresh"];
				$lifetime = $response['body']["lifetime"];

				$user->setAccessKey($this->encrypt($accessKey,$this->getFromContent("authentication")));
				$user->setAccessSecret($this->encrypt($accessSecret,$this->getFromContent("authentication")));
				$user->setRefreshSecret($this->encrypt($refreshSecret,$this->getFromContent("authentication")));
				$user->setAccessEndpoint($accessEndpoint);
				$user->setExpires(time()+$lifetime);
			}

			//If the user's username or password have changed, Update APIToken for User to Access its credentials
			if($user->isDirty("username") || $user->isDirty("authentication")){
				$searcher = new Searcher();
				$searcher->addCriterion("accessKey",Criterion::EQUAL,md5(Photographer::getSnapshot($user,"username")));
				$apiToken = $sqlGateway->findUnique("Wadapi\Authentication\APIToken",$searcher);

				$apiToken->setAccessKey(md5($user->getUsername()));
				$apiToken->setAccessSecret($user->getAuthentication());
				$apiToken->setRefreshSecret($user->getAuthentication());
				$sqlGateway->save($apiToken);
			}

			$user->setMultiSession($this->getFromContent("multi-session",false));
			$sqlGateway->save($user);

			$payload = $this->assemblePayload($user);
			ResponseHandler::modified($payload,$user->getURI());
		}

		protected function delete(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$response = $this->sendInstanceDelete($user->getAccessEndpoint());

			if($response['code'] == 200){
				CryptKeeper::bury($user);

				//Delete APIToken associated with user
				$sqlGateway = new SQLGateway();
				$searcher = new Searcher();
				$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
				$apiToken = $sqlGateway->findUnique("Wadapi\Authentication\APIToken",$searcher);
				$sqlGateway->delete($apiToken);

				ResponseHandler::deleted("User: /".RequestHandler::getRequestURI().", has been removed.");
			}else{
				ResponseHandler::error("User: /".RequestHandler::getRequestURI().", cannot be removed.");
			}
		}

		protected function isConsistent($modifiedDate,$eTag){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			return $modifiedDate == $user->getModified() && $eTag == $user->getETag();
		}
	}
?>
