<?php
	abstract class APIController extends ChanticoController{		
		protected function isInvalid(){
			$invalidArguments = array();
			
			$root = $this->getFromContent("root");
			if(!filter_var($root,FILTER_VALIDATE_URL)){
				$invalidArguments[] = "root";
			}
			
			$accessKey = $this->getFromContent("access-key");
			if(!preg_match("/^[0-9a-f]{32}$/",strtolower($accessKey))){
				$invalidArguments[] = "access-key";
			}
			
			$accessSecret = $this->getFromContent("access-secret");
			if(!preg_match("/^[0-9a-f]{32}$/",strtolower($accessSecret))){
				$invalidArguments[] = "access-secret";
			}
			
			$secure = $this->getFromContent("secure");
			if($secure && !is_bool($secure)){
				$invalidArguments[] = "secure";
			}
			
			return $invalidArguments;
		}
		
		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			return $invalidQueryParameters;
		}
		
		protected function getRecordCount(){
			$count = 0;
			$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as apis FROM API");
			$count = $countResult[0]["apis"];
			
			return $count;
		}
		
		protected function buildPageEntries($start,$count){
			$payload = array();
				
			$sqlGateway = new SQLGateway();
			
			foreach($sqlGateway->find("API",new Searcher(),new Sorter(),$count,$start,false) as $api){
				$payload[] = $this->assemblePayload($api);
			}
			
			return $payload;
		}
		
		protected function assemblePayload($api){
			$payload = array(
				"self"=>$api->getURI(),
				"root"=>$api->getRoot(),
				"secure"=>$api->isSecure()?"true":"false",
				"users"=>"{$api->getURI()}/users",
				"whitelist"=>"{$api->getURI()}/whitelist",
				"blacklist"=>"{$api->getURI()}/blacklist"
			);
			
			return $payload;
		}
	}
?>