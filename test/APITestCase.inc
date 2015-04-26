<?php		
	abstract class APITestCase extends UnitTestCase{
		protected $testBase;
		protected $apiRoot;
		protected $request;
		protected $key;
		protected $secret;
		protected $organisation;
		
		public function __construct(){
			parent::__construct();
			start_testing();
			
			$this->flushDB();
			
			$this->testBase = "chantico";
			$this->apiRoot = "localhost/{$this->testBase}";
			
			echo "Running ".get_class($this)."...\n";
		}
		
		public function __destruct(){
			stop_testing();
		}
		
		public function setup(){
			$this->initialiseRequest();
		}
		
		public function tearDown(){
		}
		
		protected function validJSON($string){
			json_decode($string);
			return (json_last_error() == JSON_ERROR_NONE);
		}
		
		protected function flushDB(){
			$databaseConnection = new MySQLi('localhost', 'root', 'c0r1nTh14n', 'sandbox');
			$databaseConnection->query("DELETE FROM chantico_Resource WHERE id NOT IN (SELECT MIN(id) FROM (SELECT * FROM chantico_Resource) as A)");
			$databaseConnection->query("DELETE FROM chantico_Warrant");
			$databaseConnection->query("DELETE FROM chantico_Renewal");
			$databaseConnection->query("DELETE FROM chantico_RenewalStub");
			$databaseConnection->commit();
			$this->key = array();
		}
		
		protected function initialiseRequest(){
			$credentials = $this->generateRootKeyPair();
			$this->request = new TestRequest("localhost","chantico");
			$this->request->authorise($credentials[0],$credentials[1]);
			$this->request->setContentType("application/json");
			$this->request->insertToHeaders("X-Test-Run","True");
		}
		
		protected function validateErrorMessage($response,$expected,$error){
			$this->assertTrue($this->validJSON($response->getBody()),"$error: Invalid JSON.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("message",$responseBody),"$error: Missing message body.");
				if(array_key_exists("message",$responseBody)){
					$this->assertEqual($expected,$responseBody["message"],"$error: Incorrect text '{$responseBody["message"]}'.");
				}
			}
		}
		
		private function generateRootKeyPair(){
			return array("test","test");
		}
	}
?>
