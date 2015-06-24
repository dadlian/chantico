<?php
	class BlacklistResource extends ChanticoController{
		protected function get(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$blacklist = $api->getBlacklist();
			$payload = $this->assemblePayload($blacklist->getEntries());
			
			ResponseHandler::retrieved($payload,$payload["self"],$blacklist->getModified(),$blacklist->getETag());
		}
		
		protected function put(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$blacklist = $api->getBlacklist();
			
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
				
			//Check for duplicates in array
			$seenIPs = array();
			foreach($this->getFromContent("ips",array()) as $ip){
				if(in_array($ip,$seenIPs)){
					ResponseHandler::conflict("The given ip list contains duplicates.");
				}else{
					$seenIPs[] = $ip;
				}
			}
			;
			$blacklist->setEntries($seenIPs);
			$api->setBlacklist($blacklist);
			
			$whitelist = $api->getWhitelist();
			$whitelist->setEntries(array_diff($whitelist->getEntries(),$seenIPs));
			$api->setWhitelist($whitelist);
			
			$sqlGateway->save($api);
			$payload = $this->assemblePayload($blacklist->getEntries());
			ResponseHandler::modified($payload,$payload["self"]);
		}
		
		protected function isInvalid(){
			$invalidArguments = array();
			if(!is_array($this->getFromContent("ips",array()))){
				$invalidArguments[] = "ips";
			}
			
			foreach($this->getFromContent("ips") as $ip){
				if(!filter_var($ip, FILTER_VALIDATE_IP)){
					$invalidArguments[] = "ips";
				}
			}
			
			return $invalidArguments;
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$blacklist = $api->getBlacklist();
			
			return $modifiedDate == $blacklist->getModified() && $eTag == $blacklist->getETag();
		}
		
		protected function assemblePayload($ipList){
			$payload = array(
				"self"=>"{$this->getBase()}/".RequestHandler::getRequestURI(),
				"total"=>sizeof($ipList),
				"ips"=>$ipList
			);
			
			return $payload;
		}
	}
?>