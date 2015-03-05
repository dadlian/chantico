<?php
	class OrganisationCasesResource extends PagedResourceController{
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
		
		protected function getRecordCount(){
			$count = 0;
			if(DatabaseAdministrator::tableExists("CaseReport")){
				$organisation = $this->getResourceObject("Organisation","slug",$this->viewFromArguments("organisation"));
				if($organisation->isGlobalAccess()){
					$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as cases FROM CaseReport");
				}else{
					$sqlGateway = new SQLGateway();
					$searcher = new Searcher();
					$searcher->addCriterion("organisations",Criterion::INCLUDES,$organisation);
					$groupIds = array();
					foreach($sqlGateway->find("OrganisationGroup",$searcher) as $group){
						$groupIds[] = $group->getId();
					}
					
					$query = "SELECT COUNT(id) as cases FROM CaseReport WHERE filedBy = {$organisation->getId()}";
					if($groupIds){
						$query .= " OR id IN (SELECT casereport FROM CaseReportReferees WHERE value IN (".implode(",",$groupIds)."))";
					}
					
					$countResult = DatabaseAdministrator::execute($query);
				}
				$count = $countResult[0]["cases"];
			}
			
			return $count;
		}
		
		protected function buildPageEntries($start,$count){
			$payload = array();
			
			$sqlGateway = new SQLGateway();
			$organisation = $this->getResourceObject("Organisation","slug",$this->viewFromArguments("organisation"));
			if($organisation->isGlobalAccess()){
				$cases = $sqlGateway->find("Cases",new Searcher(),new Sorter(),$count,$start);
			}else{
				$sqlGateway = new SQLGateway();
				$searcher = new Searcher();
				$searcher->addCriterion("organisations",Criterion::INCLUDES,$organisation);
				$groupIds = array();
				foreach($sqlGateway->find("OrganisationGroup",$searcher) as $group){
					$groupIds[] = $group->getId();
				}
				
				$query = "SELECT id FROM CaseReport WHERE filedBy = {$organisation->getId()}";
				if($groupIds){
					$query .= " OR id IN (SELECT casereport FROM CaseReportReferees WHERE value IN (".implode(",",$groupIds)."))";
				}
				
				$caseIds = array();
				foreach(DatabaseAdministrator::execute($query) as $caseResult){
					$caseIds[] = $caseResult['id'];
				}
				
				$searcher->clearCriteria();
				$searcher->addCriterion("id",Criterion::EQUAL,$caseIds);
				$cases = $sqlGateway->find("CaseReport",$searcher,new Sorter(),$count,$start);
			}
			
			foreach($cases as $case){
				$payload[] = $this->assemblePayload($case);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($case){
			$referees = array();
			foreach($case->getReferees() as $referee){
				$referees[] = $referee->getURI();
			}
			
			$payload = array(
				"self"=>$case->getURI(),
				"number"=>$case->getNumber(),
				"filed-by"=>$case->getFiledBy(),
				"referees"=>$referees,
				"reported-on"=>date("c",$case->getCreated()),
				"reported-by"=>$case->getReportedBy(),
				"status"=>$case->getStatus(),
				"victim"=>"{$case->getURI()}/victim",
				"perpetrators"=>"{$case->getURI()}/perpetrators",
				"incident"=>"{$case->getURI()}/incident",
				"checklist"=>"{$case->getURI()}/checklist",
				"notes"=>"{$case->getURI()}/notes",
				"log"=>"{$case->getURI()}/log"
			);
			
			return $payload;
		}
	}
?>