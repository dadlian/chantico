<?php
	class UserRenewalsCollection extends UserController{
		protected function isInvalid(){
			return array();
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
		
		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			
			$warrant = RequestHandler::getQueryParameter("warrant");
			if($warrant){
				$sqlGateway = new SQLGateway();
				$searcher = new Searcher();
				$searcher->addCriterion("token",Criterion::EQUAL,md5($warrant.$_SERVER['REMOTE_ADDR']));
				
				if(!$sqlGateway->findUnique("RenewalStub",$searcher) || !Magistrate::executeWarrant($warrant)){
					$invalidQueryParameters[] = "warrant";
				}
			}
			
			return $invalidQueryParameters;
		}
		
		protected function getRecordCount(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			return sizeof($user->getRenewals());
		}
		
		protected function buildPageEntries($start,$count){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$payload = array();
			
			foreach(array_slice($user->getRenewals(),($start-1)*$count,$count) as $renewal){
				$payload[] = $this->assemblePayload($renewal);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($renewal){
			$payload = array(
				"renewed-at"=>date("c",$renewal->getRenewedAt())
			);
			
			return $payload;
		}
		
		protected function post(){
			$invalidArguments = $this->getInvalidQueryParameters();
			if($invalidArguments){
				ResponseHandler::bad("The following query parameters have invalid values: ".implode(", ",$invalidArguments).".");
			}
			
			if(!RequestHandler::getQueryParameter("warrant")){
				ResponseHandler::bad("The following query parameters are required, but missing: warrant.");
			}
			
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			//Check that a renewal request is in time
			if($user->getExpires() < time()){
				ResponseHandler::forbidden("The user tokens have expired and can no longer be renewed.");
			}
			
			//Retrieve user access resource
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			$request->setEndpoint($user->getAccessEndpoint());
			$request->authorise($api->getAccessKey(),$api->getAccessSecret());
			$request->setContentType("application/json");
			$response = $request->get();
			
			if($response->getStatusCode() != 200){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}else{
				$access = json_decode($response->getBody(),true);
			}
			
			//Retrieve Renewal Stub
			$token = RequestHandler::getQueryParameter("warrant").$_SERVER['REMOTE_ADDR'];
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("token",Criterion::EQUAL,md5($token));
			$stub = $sqlGateway->findUnique("RenewalStub",$searcher);
			
			//Refresh the user token
			$accessEndpoint = parse_url($access["tokens"],PHP_URL_PATH);
			$request->setEndpoint($accessEndpoint);
			$request->authorise($this->decrypt($stub->getAccessKey(),$token),$this->decrypt($stub->getRefreshSecret(),$token));
			$request->setBody(json_encode(array("lifetime"=>SettingsManager::getSetting("options","lifetime"))));
			$response = $request->post();
			if($response->getStatusCode() == 201){
				$body = json_decode($response->getBody(),true);
				$accessKey = $body["key"];
				$accessSecret = $body["secret"];
				$refreshSecret = $body["refresh"];
				$lifetime = $body["lifetime"];
				
				//Save User to Chantico
				$password = $this->decrypt($stub->getUserPassword(),$token);
				$user->setAccessKey($this->encrypt($accessKey,$password));
				$user->setAccessSecret($this->encrypt($accessSecret,$password));
				$user->setRefreshSecret($this->encrypt($refreshSecret,$password));
				$user->setExpires(time()+$lifetime);
			}else{
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}
			
			//Register this renewal event
			$user->appendToRenewals(new Renewal($user->getModified()));
			$sqlGateway->save($user);
			
			//Update the renewal stub
			$warrant = Magistrate::issueWarrant("renew");
			$stubToken = $warrant.$_SERVER['REMOTE_ADDR'];
			$stub->setToken(md5($stubToken));
			$stub->setAccessKey($this->encrypt($accessKey,$stubToken));
			$stub->setRefreshSecret($this->encrypt($refreshSecret,$stubToken));
			$stub->setUserPassword($this->encrypt($password,$stubToken));
			$sqlGateway->save($stub);
			
			$uri = ($_SERVER['HTTPS']?"https://":"http://").SettingsManager::getSetting("install","urlroot")."/".RequestHandler::getRequestURI();
			$payload = array(
				"key"=>$accessKey,
				"secret"=>$accessSecret,
				"expiry"=>$user->getExpires() - time(),
				"renew"=>$user->getURI()."/renewals?warrant=$warrant"
			);
			
			ResponseHandler::created($payload,$uri);
		}
	}
?>