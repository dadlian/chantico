<?php
	use Wadapi\Persistence\DatabaseAdministrator;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Sorter;
	use Wadapi\Persistence\Criterion;

	abstract class UsernameRecoveryController extends ChanticoController{
		protected function isInvalid(){
			$invalidArguments = array();
			return $invalidArguments;
		}

		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			return $invalidQueryParameters;
		}

		protected function getRecordCount(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$count = sizeof($user->getUsernameRecoveries());

			return $count;
		}

		protected function buildPageEntries($start,$count){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$recoveries = array();
			foreach($user->getUsernameRecoveries() as $recovery){
				$recoveries[$recovery->getId()] = $recovery;
			}

			krsort($recoveries);

			$payload = array();
			foreach(array_slice($recoveries,$start,$count) as $recovery){
				$payload[] = $this->assemblePayload($recovery);
			}

			return $payload;
		}

		protected function assemblePayload($recovery){
			$payload = array(
				"self"=>$recovery->getURI(),
				"recovered"=>date("c",$recovery->getCreated()),
			);

			return $payload;
		}
	}
?>
