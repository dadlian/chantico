<?php
	class OrganisationCollection extends OrganisationController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("name",Criterion::EQUAL,$this->getFromContent("name"));
			if($sqlGateway->findUnique("Organisation",$searcher)){
				ResponseHandler::conflict("An organisation named '{$this->getFromContent("name")}' already exists.");
			}
			
			$slug = strtolower(preg_replace("/\s/","",RequestHandler::getSuggestedURI()));
			$organisation = new Organisation($slug);
			$organisation->reconcileSlug();
			$organisation->setName($this->getFromContent("name"));
			$organisation->setActive($this->getFromContent("active",true));
			
			$sqlGateway = new SQLGateway();
			$sqlGateway->save($organisation);
			
			$payload = $this->assemblePayload($organisation);
			ResponseHandler::created($payload,$organisation->getURI());
		}
		
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>