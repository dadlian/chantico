<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;

	class WhitelistResource extends ChanticoController{
		public function execute(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("title",Criterion::EQUAL,"whitelist");
			$whitelist = $sqlGateway->findUnique("IPList",$searcher);

			if(!$whitelist){
				$sqlGateway->save(new IPList("whitelist"));
			}

			$searcher->clearCriteria();
			$searcher->addCriterion("title",Criterion::EQUAL,"blacklist");
			$blacklist = $sqlGateway->findUnique("IPList",$searcher);

			if(!$blacklist){
				$sqlGateway->save(new IPList("blacklist"));
			}

			parent::execute();
		}

		protected function get(){
			$whitelist = $this->getResourceObject("IPList","title","whitelist");

			$payload = $this->assemblePayload($whitelist->getEntries());
			ResponseHandler::retrieved($payload,$payload["self"],$whitelist->getModified(),$whitelist->getETag());
		}

		protected function put(){
			$whitelist = $this->getResourceObject("IPList","title","whitelist");

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
			$blacklist = $this->getResourceObject("IPList","title","blacklist");
			$blacklist->setEntries(array_diff($blacklist->getEntries(),$seenIPs));
			$sqlGateway->save([$whitelist,$blacklist]);

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
			$whitelist = $this->getResourceObject("IPList","title","whitelist");
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

		protected function getRecordCount(){
			return 0;
		}

		protected function buildPageEntries($start,$count){
			return array();
		}
	}
?>
