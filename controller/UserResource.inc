<?php
	class OrganisationResource extends OrganisationController{
		protected function get(){
			$organisation = $this->getResourceObject("Organisation","slug",$this->viewFromArguments("organisation"));
			$payload = $this->assemblePayload($organisation);
			ResponseHandler::retrieved($payload,$organisation->getURI(),$organisation->getModified(),$organisation->getETag());
		}
		
		protected function put(){
			$organisation = $this->getResourceObject("Organisation","slug",$this->viewFromArguments("organisation"));
		
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("name",Criterion::EQUAL,$this->getFromContent("name"));
			$searcher->addCriterion("id",Criterion::NOT_EQUAL,$organisation->getId());
			if($sqlGateway->findUnique("Organisation",$searcher)){
				ResponseHandler::conflict("An organisation named '{$this->getFromContent('name')}' already exists.");
			}
		
			$organisation->setName($this->getFromContent("name"));
			$organisation->setActive($this->getFromContent("active",true));
			
			$sqlGateway = new SQLGateway();
			$sqlGateway->save($organisation);
			
			$payload = $this->assemblePayload($organisation);
			ResponseHandler::modified($payload,$organisation->getURI());
		}
		
		protected function delete(){
			$organisation = $this->getResourceObject("Organisation","slug",$this->viewFromArguments("organisation"));
			
			//Prevent deletion of organisations with filed cases
			$caseResult = DatabaseAdministrator::execute("SELECT COUNT(id) as cases FROM CaseReport WHERE filedBy = ?",$organisation->getId());
			$cases = $caseResult[0]["cases"];
			if($cases > 0){
				ResponseHandler::conflict("Organisation: /".RequestHandler::getRequestURI()." has $cases filed case".(($cases>1)?"s":"")." and cannot be deleted.");
			}
			
			CryptKeeper::bury($organisation);
			ResponseHandler::deleted("Organisation: /".RequestHandler::getRequestURI().", has been removed.");
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			$organisation = $this->getResourceObject("Organisation","slug",$this->viewFromArguments("organisation"));
			return $modifiedDate == $organisation->getModified() && $eTag == $organisation->getETag();
		}
	}
?>