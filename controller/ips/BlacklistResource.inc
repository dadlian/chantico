<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;

	class BlacklistResource extends ChanticoController{
		public function execute(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("title",Criterion::EQUAL,"blacklist");
			$blacklist = $sqlGateway->findUnique("IPList",$searcher);

			if(!$blacklist){
				$sqlGateway->save(new IPList("blacklist"));
			}

			$searcher->clearCriteria();
			$searcher->addCriterion("title",Criterion::EQUAL,"whitelist");
			$whitelist = $sqlGateway->findUnique("IPList",$searcher);

			if(!$whitelist){
				$sqlGateway->save(new IPList("whitelist"));
			}

			parent::execute();
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

		protected function getRecordCount(){
			return 0;
		}

		protected function buildPageEntries($start,$count){
			return array();
		}
	}
?>
