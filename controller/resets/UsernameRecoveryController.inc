<?php
	use Wadapi\Persistence\DatabaseAdministrator;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Sorter;
	use Wadapi\Persistence\Criterion;

	abstract class UsernameRecoveryController extends ChanticoController{
		protected function isInvalid(){
			$invalidArguments = array();

			$recipient = $this->getFromContent("recipient");
			if($recipient && !is_string($recipient)){
				$invalidArguments[] = "recipient";
			}

			$email = $this->getFromContent("email");
			if($email && !filter_var($email,FILTER_VALIDATE_EMAIL)){
				$invalidArguments[] = "email";
			}

			return $invalidArguments;
		}

		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			return $invalidQueryParameters;
		}

		protected function getRecordCount(){
			$count = 0;
			$user = $this->viewFromArguments("user");
			$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as recovery FROM UsernameRecovery WHERE user = $user");
			$count = $countResult[0]["recovery"];

			return $count;
		}

		protected function buildPageEntries($start,$count){
			$payload = array();
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("user",Criterion::EQUAL,$this->viewFromArguments("user"));
			$sorter = new Sorter();
			$sorter->addCriterion("id",Criterion::DESCENDING);

			foreach($sqlGateway->find("UsernameRecovery",$searcher,$sorter,$count,$start) as $reset){
				$payload[] = $this->assemblePayload($reset);
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
