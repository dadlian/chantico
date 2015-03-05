<?php
	abstract class UserController extends PagedResourceController{
		protected function isInvalid(){
			$invalidArguments = array();
			
			if(!is_string($this->getFromContent("username"))){
				$invalidArguments[] = "username";
			}
			
			if(!preg_match("/[a-f0-9]{32}/",$this->getFromContent("authentication"))){
				$invalidArguments[] = "authentication";
			}
			
			if($this->getFromContent("role") && !is_string($this->getFromContent("role"))){
				$invalidArguments[] = "role";
			}
			
			if(!is_array($this->getFromContent("options"))){
				$invalidArguments[] = "options";
			}
			
			$options = $this->getFromContent("options")?$this->getFromContent("options"):array();
			foreach($options as $option){
				if(!is_string($option)){
					$invalidArguments[] = "options";
					break;
				}
			}
			
			return $invalidArguments;
		}
		
		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			return $invalidQueryParameters;
		}
		
		protected function getRecordCount(){
			$count = 0;
			if(DatabaseAdministrator::tableExists("ManagedAccess")){
				$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess");
			}
			
			$count = $countResult[0]["user"];
			return $count;
		}
		
		protected function buildPageEntries($start,$count){
			$payload = array();
				
			$sqlGateway = new SQLGateway();
			foreach($sqlGateway->find("ManagedAccess") as $user){
				$payload[] = $this->assemblePayload($user);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($user){
			$request = new WadapiRequest(SettingsManager::getSetting("options","api"),"access");
			$request->authorise(SettingsManager::getSetting("options","key"),SettingsManager::getSetting("options","secret"));
			$request->setContentType("application/json");
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
	}
?>