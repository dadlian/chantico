#!/usr/bin/php
<?php
	require_once(dirname(__FILE__).'/../wadadli/third_party/simpletest/autorun.php');
	
	require_once(dirname(__FILE__).'/../wadadli/lib/toolbox/files.php');
	require_all(dirname(__FILE__).'/../wadadli/lib/toolbox');
	require_all(dirname(__FILE__).'/../wadadli/lib/worker');
	require_all(dirname(__FILE__).'/../wadadli/third_party/addendum');
	require_all(dirname(__FILE__).'/../wadadli/lib/model');
	require_all(dirname(__FILE__).'/../wadadli/modules/wadapi');
	
	require_once(dirname(__FILE__).'/TestRequest.inc');
	require_once(dirname(__FILE__).'/APITestCase.inc');
	
	class AllTests extends TestSuite{
		function __construct(){
			parent::__construct();
			$this->addFile(dirname(__FILE__).'/user_tests.php');
			//$this->addFile(dirname(__FILE__).'/ip_tests.php');
		}
	}
?>