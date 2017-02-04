<?php
	class RenewalStub extends PersistentClass{
		/** @WadapiString(max=1024) */
		protected $token;
		
		/** @WadapiString(max=1024) */
		protected $accessKey;
		
		/** @WadapiString(max=1024) */
		protected $refreshSecret;
		
		/** @WadapiString(max=1024) */
		protected $userPassword;
	}
?>