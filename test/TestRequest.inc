<?php
	class TestRequest extends WadapiRequest{
		/** @String(default="") */
		protected $testBase;
		
		public function __construct($host,$initialTestBase){
			parent::__construct($host);
			$this->setTestBase($initialTestBase);
		}
	
		function setEndpoint($newEndpoint){
			$newEndpoint = $this->getTestBase()?"{$this->getTestBase()}/$newEndpoint":$newEndpoint;
			parent::setEndpoint($newEndpoint);
		}
	}
?>