<?php
	class RenewalStub extends PersistentClass{
		/** @String(max=1024) */
		protected $token;
		
		/** @String(max=1024) */
		protected $accessKey;
		
		/** @String(max=1024) */
		protected $refreshSecret;
		
		/** @String(max=1024) */
		protected $userPassword;
	}
?>