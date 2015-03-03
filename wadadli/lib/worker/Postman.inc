<?php
	class Postman extends Worker{
		//An associative array where each entry is an array of messages destined for a specific class
		private static $postOffice = array();
		
		//An SQLGateway for writing undelivered messages to the DB
		private static $gateway = null;
		
		protected static function hasMessages(){
			$hasMessages = false;
			foreach(self::$postOffice as $mailbox){
				$hasMessages = $hasMessages || $mailbox;
			}
			
			return $hasMessages;
		}
			
		protected static function postSuccess($message, $recipient = "*"){
			return self::postMessage(WadadliMessage::SUCCESS, $message, $recipient);
		}
		
		protected static function postWarning($message, $recipient = "*"){
			return self::postMessage(WadadliMessage::WARNING, $message, $recipient);
		}
		
		protected static function postError($message, $recipient = "*"){
			return self::postMessage(WadadliMessage::ERROR, $message, $recipient);
		}
		
		protected static function postDebug($message, $recipient = "*"){
			return self::postMessage(WadadliMessage::DEBUG, $message, $recipient);
		}
		
		private static function postMessage($messageType, $message, $recipient){
			$sender = get_caller(4);
			if(!in_array($recipient, array_keys(self::$postOffice))){
				self::$postOffice[$recipient] = array();
			}
			
			$wadadliMessage = new WadadliMessage(session_id(), $messageType, $message, $sender, $recipient);
			self::$postOffice[$recipient][$wadadliMessage->getId()] = $wadadliMessage;
		}
		
		protected static function deliverSuccesses($index = null){
			$recipient = get_caller(3);
			return self::deliverMessages($recipient, WadadliMessage::SUCCESS, $index);
		}
		
		protected static function deliverWarnings($index = null){
			$recipient = get_caller(3);			
			return self::deliverMessages($recipient, WadadliMessage::WARNING, $index);
		}
		
		protected static function deliverErrors($index = null){
			$recipient = get_caller(3);
			return self::deliverMessages($recipient, WadadliMessage::ERROR, $index);
		}
		
		protected static function deliverDebugs($index = null){
			$recipient = get_caller(3);
			return self::deliverMessages($recipient, WadadliMessage::DEBUG, $index);
		}
		
		protected static function discardPost(){
			self::$postOffice = array();
		}
		
		private static function deliverMessages($recipient, $messageType, $index){
			$recipientMessages = array();
			if(array_key_exists($recipient, self::$postOffice)){
				foreach(self::$postOffice[$recipient] as $message){
					if($message->getType() == $messageType){
						$recipientMessages[] = $message;
					}
				}
			}
			
			$globalMessages = array();
			if(array_key_exists("*", self::$postOffice)){
				foreach(self::$postOffice["*"] as $message){
					if($message->getType() == $messageType){
						$globalMessages[] = $message;
					}
				}
			}
			
			if(is_null($index)){
				foreach($recipientMessages as $message){
					unset(self::$postOffice[$recipient][$message->getId()]);
				}
				
				foreach($globalMessages as $message){
					unset(self::$postOffice["*"][$message->getId()]);
				}
				
				return array_merge($recipientMessages, $globalMessages);
			}
			
			$allMessages = array_merge($recipientMessages, $globalMessages);
			if(array_key_exists($index, $allMessages)){
				$message = $allMessages[$index];
				
				if($recipientMessages && array_key_exists($message->getId(), self::$postOffice[$recipient])){
					unset(self::$postOffice[$recipient][$message->getId()]);
				}else{
					unset(self::$postOffice["*"][$message->getId()]);
				}
				
				return $message;
			}else{
				return new WadadliMessage(session_id(),$messageType, "", "", "");
			}
		}
		
		/*
		 * Saves a copy of all Undelivered Messages to the DB for future use
		 */
		protected static function saveMessages(){
			if(!self::$gateway){
				self::$gateway = new SQLGateway();
			}
			
			foreach(self::$postOffice as $recipient => $mailbox){
				foreach($mailbox as $id => $message){
					if($message->getText()){
						self::$gateway->save($message);
					}
				}
			}
		}
		
		/*
		 * Loads all saved messages for the current session into memory and deletes them from the database
		 */
		protected static function loadMessages(){
			$searcher = new Searcher();
			$sqlGateway = new SQLGateway();
			$searcher->addCriterion("session",Criterion::EQUAL,session_id());
			$messages = $sqlGateway->find("WadadliMessage",$searcher);
			
			foreach($messages as $message){
				if(!array_key_exists($message->getRecipient(), self::$postOffice)){
					self::$postOffice[$message->getRecipient()] = array();
				}
				
				self::$postOffice[$message->getRecipient()][$message->getId()] = $message;
				$sqlGateway->delete($message);
			}
		}
	}
?>