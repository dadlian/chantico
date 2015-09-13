#!/usr/bin/php
<?php
	require_once(dirname(__FILE__).'/TestResponse.inc');
	require_once(dirname(__FILE__).'/TestRequest.inc');
	
	require_once(dirname(__FILE__).'/simpletest/autorun.php');
	require_once(dirname(__FILE__).'/APITestCase.inc');
	
	class AllTests extends TestSuite{
		function __construct(){
			parent::__construct();
			$this->addFile(dirname(__FILE__).'/user_tests.php');
			//->addFile(dirname(__FILE__).'/ip_tests.php');
		}
	}
?>