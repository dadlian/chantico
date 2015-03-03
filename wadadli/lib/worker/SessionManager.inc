<?php
	class SessionManager extends Worker{
		private static $activeSession = null;
		
		public static function startSession(){
			if(self::$activeSession){
				return;
			}
			
			if(!is_test_run()){
				//Configure Session
				ini_set( 'session.cookie_httponly', 1 );
				
				session_start();
			}
			
			$searcher = new Searcher();
			$searcher->addCriterion('sessionId',Criterion::EQUAL,session_id());
			
			$sqlGateway = new SQLGateway();
			$savedSession = $sqlGateway->findUnique('Session',$searcher);
			
			if($savedSession){
				self::$activeSession = $savedSession;
			}else{
				self::$activeSession = new Session(session_id(),$_SERVER['REMOTE_ADDR'],null);
			}
		}
		
		public static function endSession(){
			self::$activeSession = null;
		}
		
		public static function activeUser(){
			if(!self::$activeSession){
				warning(RESOURCE_NOT_READY_WARNING, "SessionManager cannot return the active user of an uninitialised Session.");
				return;
			}
			
			return self::$activeSession->getActiveUser();
		}
		
		public static function changeUser($newUser){
			if(!is_object($newUser) || get_class($newUser) != "User"){
				$type = gettype($newUser);
				if($type == 'object'){
					$type = get_class($newUser);
				}
				
				warning(UNEXPECTED_ARGUMENT_WARNING, "SessionManager changeUser expects a User object argument, $type given.");
				return;
			}
			
			if(!self::$activeSession){
				warning(RESOURCE_NOT_READY_WARNING, "SessionManager cannot change the active user of an uninitialised Session.");
				return;
			}
			
			$searcher = new Searcher();
			$searcher->addCriterion('activeUser',Criterion::EQUAL,$newUser);
			$searcher->addCriterion('ipAddress',Criterion::EQUAL,$_SERVER['REMOTE_ADDR']);
			
			$sqlGateway = new SQLGateway();
			$sqlGateway->delete($sqlGateway->find('Session',$searcher));
			
			self::$activeSession->setActiveUser($newUser);
			self::save();
		}
		
		public static function save(){
			if(!self::$activeSession){
				warning(RESOURCE_NOT_READY_WARNING, "SessionManager cannot save an uninitialised Session.");
				return;
			}
			
			$gateway = new SQLGateway();
			$gateway->save(self::$activeSession);
		}
		
		public static function reset(){
			if(!self::$activeSession){
				warning(RESOURCE_NOT_READY_WARNING, "SessionManager cannot reset an uninitialised Session.");
				return;
			}	
			
			$gateway = new SQLGateway();
			$gateway->delete(self::$activeSession);
			
			self::$activeSession->setActiveUser(null);
		}
	}
?>