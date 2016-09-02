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
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			$sqlGateway = new SQLGateway();
			$authentication = RequestHandler::getAuthorisation();
			$username = $authentication["key"];
			$password = $authentication["secret"];
			
			//Retrieve user access resource
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			$request->setTransport("ssl");
			$request->setPort(443);
			$request->setEndpoint($user->getAccessEndpoint());
			$request->authorise($api->getAccessKey(),$api->getAccessSecret());
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
					
					$sqlGateway->save($user);
				}
			}
			
			//Invalidate inactive renewal warrants
			$searcher = new Searcher();
			$searcher->addCriterion("action",Criterion::EQUAL,"renew");
			$searcher->addCriterion("executed",Criterion::IS,null);
			foreach($sqlGateway->find("Warrant",$searcher) as $warrant){
				//Remove unused renewal stubs
				$searcher->clearCriteria();
				$searcher->addCriterion("token",Criterion::EQUAL,md5($warrant->getCode().$_SERVER['REMOTE_ADDR']));
				if($stub = $sqlGateway->findUnique("RenewalStub",$searcher)){
					Magistrate::executeWarrant($warrant->getCode());
					$sqlGateway->delete($stub);
				}
			}
			
			//Generate a new renewal stub
			$warrant = Magistrate::issueWarrant("renew");
			
			$stubToken = $warrant.$_SERVER['REMOTE_ADDR'];
			$accessKey = $this->encrypt($this->decrypt($user->getAccessKey(),$password),$stubToken);
			$refreshSecret = $this->encrypt($this->decrypt($user->getRefreshSecret(),$password),$stubToken);
			$userPassword = $this->encrypt($password,$stubToken);
			$stub = new RenewalStub(md5($stubToken),$accessKey,$refreshSecret,$userPassword);
			$sqlGateway->save($stub);
			
			$payload = array(
				"self"=>$this->getBase()."/".RequestHandler::getRequestURI(),
				"key"=>$this->decrypt($user->getAccessKey(),$password),
				"secret"=>$this->decrypt($user->getAccessSecret(),$password),
				"expiry"=>$user->getExpires()-time(),
				"renew"=>$user->getURI()."/renewals?warrant=$warrant"
			);
			
			ResponseHandler::retrieved($payload,$this->getBase()."/".RequestHandler::getRequestURI(),$user->getModified(),$user->getETag());
		}
	}
?>