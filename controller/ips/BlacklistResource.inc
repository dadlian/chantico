<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;

	class BlacklistResource extends ChanticoController{
		protected function execute(){
			parent::execute();

			$sqlGateway = new SQLGateway();

			$blacklist = $this->getResourceObject("IPList","title","blacklist");
			if(!$whitelist){
				$sqlGateway->save(new IPList("blacklist"));
			}

			$whitelist = $this->getResourceObject("IPList","title","whitelist");
			if(!$whitelist){
				$sqlGateway->save(new IPList("whitelist"));
			}
		}

		protected function get(){
			$blacklist = $this->getResourceObject("IPList","title","blacklist");

			$payload = $this->assemblePayload($blacklist->getEntries());
			ResponseHandler::retrieved($payload,$payload["self"],$blacklist->getModified(),$blacklist->getETag());
		}

		protected function put(){
			$blacklist = $this->getResourceObject("IPList","title","blacklist");

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

			$blacklist->setEntries($seenIPs);
			$whitelist = $this->getResourceObject("IPList","title","whitelist");
			$whitelist->setEntries(array_diff($whitelist->getEntries(),$seenIPs));
			$sqlGateway->save([$blacklist,$whitelist]);

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
			$blacklist = $this->getResourceObject("IPList","title","blacklist");
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
