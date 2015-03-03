<?php
	class WadadliMessage extends PersistentClass{
		const ERROR = 'error';
		const WARNING = 'warning';
		const SUCCESS = 'success';
		const DEBUG = 'debug';	
		
		//The session in which the message was generated for persistent delivery
		/** @String(required=true) */
		protected $session;
		
		//The message type. (Error, warning, success, etc.)
		/** @String(required=true, values={'error', 'warning', 'success', 'debug'}) */
		protected $type;
		
		//The text associated with this message
		/** @Text(required=true) */
		protected $text;
		
		//The message sender
		/** @String(required=true) */
		protected $sender;
		
		//The intended message recipient
		/** @String */
		protected $recipient;
	}
?>