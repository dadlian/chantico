<?php
	class Test{
		public static $isTestRun = false;
	}
	
	function start_testing(){
		Test::$isTestRun = true;
	}
	
	function stop_testing(){
		Test::$isTestRun = false;
	}
	
	function is_test_run(){
		return Test::$isTestRun;
	}
?>