<?php
	class WhitelistResource extends ChanticoController{
		protected function get(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$whitelist = $api->getWhitelist();
			
			$payload = $this->assemblePayload($whitelist->getEntries());
			
			ResponseHandler::retrieved($payload,$payload["self"],$whitelist->getModified(),$whitelist->getETag());
		}
		
		protected function put(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$whitelist = $api->getWhitelist();
			
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
			
			$whitelist->setEntries($seenIPs);
			$api->setWhitelist($whitelist);
			
			$blacklist = $api->getBlacklist();
			$blacklist->setEntries(array_diff($blacklist->getEntries(),$seenIPs));
			$api->setBlacklist($blacklist);
			
			$sqlGateway->save($api);
			$payload = $this->assemblePayload($whitelist->getEntries());
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
			$whitelist = $api->getWhitelist();
			
			return $modifiedDate == $whitelist->getModified() && $eTag == $whitelist->getETag();
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