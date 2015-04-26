<?php
	class Magistrate extends Worker{
		public static function issueWarrant($action){
			$code = md5($action.time().rand());
			$sqlGateway = new SQLGateway();
			$sqlGateway->save(new Warrant($code,$action));
			
			return $code;
		}
	}
?>