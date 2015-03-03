<?php
	abstract class ResourceController extends Controller{
		public function execute(){
			$method = strtolower(RequestHandler::getMethod());
			
			if(!Mirror::reflectClass($this)->hasMethod($method)){
				ResponseHandler::unsupported("/".RequestHandler::getRequestURI()." does not support the ".RequestHandler::getMethod()." method.");
			}else{
				if(in_array($method,array("post","put"))){
					//Check for missing and invalid arguments
					$this->ensureRequirements();
					$this->validateArguments();
				}
				
				if(in_array($method,array("put","delete")) && $this->mustPrevalidate()){
					if(RequestHandler::checksConsistency()){
						//Check for consistency preconditions
						$consistencyTags = RequestHandler::getConsistencyTags();
						if(!$this->isConsistent($consistencyTags["ifUnmodifiedSince"],$consistencyTags["ifMatch"])){
							ResponseHandler::precondition("The wrong Modification Date and ETag values were given for this resource.");
						}
					}else{
						ResponseHandler::forbidden("If-Unmodified-Since and If-Match Headers must be specified to modify this resource.");
					}
				}
				
				$this->$method();
			}
		}
		
		protected function getBase(){
			return "http://".SettingsManager::getSetting("install","urlroot");
		}
		
		protected function getFromContent($argument,$default=""){
			$arguments = RequestHandler::getContent();
			return array_key_exists($argument,$arguments)?$arguments[$argument]:$default;
		}
		
		private function ensureRequirements(){
			$missingArguments = array();
			foreach(RequestHandler::getEndpoint()->getRequirements() as $field){
				if(!array_key_exists($field,RequestHandler::getContent())){
					$missingArguments[] = $field;
				}
			}
			
			if($missingArguments){
				ResponseHandler::bad("The following arguments are required, but have not been supplied: ".implode(", ",$missingArguments).".");
			}
		}
		
		protected function validateArguments(){
			if($invalidArguments = $this->isInvalid()){
				ResponseHandler::bad("The following arguments have invalid values: ".implode(", ",$invalidArguments).".");
			}
		}
		
		protected function getResourceObject($resourceClass,$resourceKeyField,$resourceKeyValue){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion($resourceKeyField,Criterion::EQUAL,$resourceKeyValue);
			$resource = $sqlGateway->findUnique($resourceClass,$searcher);
			
			if(!$resource && !CryptKeeper::exhume($resourceKeyValue,$resourceClass)){
				ResponseHandler::missing("There is presently no resource with the given URI.");
			}
			
			return $resource;
		}
		
		protected function mustPrevalidate(){
			return true;
		}
		
		protected abstract function isInvalid();
		protected abstract function isConsistent($modified,$eTag);
		protected abstract function assemblePayload($object);
	}
?>