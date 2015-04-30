<?php
	abstract class IPListResource extends ChanticoController{
		protected function loadIPList($listTitle){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("title",Criterion::EQUAL,$listTitle);
			
			$ipList = $sqlGateway->findUnique("IPList",$searcher);
			if(!$ipList){
				$ipList = new IPList($listTitle);
			}
			$sqlGateway->save($ipList);
			
			return $ipList;
		}
	}
?>