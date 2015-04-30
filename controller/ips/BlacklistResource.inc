<?php
	class BlacklistResource extends IPListResource{
		protected function get(){
			$blacklist = $this->loadIPList("blacklist");
			$payload = $this->assemblePayload($blacklist->getEntries());
			
			ResponseHandler::retrieved($payload,$payload["self"],$blacklist->getModified(),$blacklist->getETag());
		}
		
		protected function put(){
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
			
			$blacklist = $this->loadIPList("blacklist");
			$blacklist->setEntries($seenIPs);
			$sqlGateway->save($blacklist);
			
			$whitelist = $this->loadIPList("whitelist");
			$whitelist->setEntries(array_diff($whitelist->getEntries(),$seenIPs));
			$sqlGateway->save($whitelist);
			
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
			$blacklist = $this->loadIPList("blacklist");
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