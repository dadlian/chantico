<?php		
	abstract class APITestCase extends UnitTestCase{
		protected $apiRoot;
		protected $testPath;
		protected $request;
		protected $apiTestKey;
		protected $apiTestSecret;
		private $testers;
		
		public function __construct(){
			parent::__construct();
			
			$this->apiRoot = "login.sandbox.mywadapi.com";
			$this->testPath = dirname(__FILE__);
			$this->request = new TestRequest($this->apiRoot);
			$this->apiTestKey = "";
			$this->apiTestSecret = "";
			
			$this->testers = array(
				"root"=>array("key"=>"da10b82595a1105718a74ea27948830e","secret"=>"e0d258e38a5e698ffb48e90ddacfb0a7"),
				"administrator"=>array("key"=>"7d01b82d2aef619eab506ee0a7844fd2","secret"=>"495439acbda183979efe1414b98b5525"),
				"user"=>array("key"=>"a0d32e879fae236bcff8f01bebb02ba1","secret"=>"2b18d1f105137c26fe8272090d411e94")
			);
			
			echo "Running ".get_class($this)."...\n";
		}
		
		public function setup(){
			$this->initialiseRequest();
		}
		
		public function teardown(){
			$this->request->flushTestResources($this->testers["root"]["key"],$this->testers["root"]["secret"]);
		}
		
		protected function requestAs($role){
			if(array_key_exists($role,$this->testers)){
				$this->request->authorise($this->testers[$role]["key"],$this->testers[$role]["secret"]);
			}else{
				$this->request->takeFromHeaders("Authorization");
			}
		}
		
		protected function initialiseRequest(){
			if($this->request){
				$this->request->flushTestResources($this->testers["root"]["key"],$this->testers["root"]["secret"]);
			}
		
			$this->request = new TestRequest($this->apiRoot);
			$this->requestAs("root");
			$this->request->setContentType("application/json");
		}
		
		protected function validJSON($string){
			json_decode($string);
			return (json_last_error() == JSON_ERROR_NONE);
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
		
		protected function createAccessToken($role,$authorise=true){
			//Create Access Token Resource
			$this->requestAs("login");
			$this->request->setEndpoint("/access");
			$this->request->setBody(json_encode(array("role"=>$role)));
			$response = $this->request->post();
			$access = str_replace("http://{$this->apiRoot}","",$response->viewFromHeaders("Location"));
			$responseBody = json_decode($response->getBody(),true);
			
			$lifetime = 3600;
			$this->request->setEndpoint("$access/tokens");
			$key = (array_key_exists("active-token",$responseBody) && array_key_exists("key",$responseBody['active-token']))?$responseBody["active-token"]["key"]:"";
			$refresh = (array_key_exists("active-token",$responseBody) && array_key_exists("refresh",$responseBody['active-token']))?$responseBody["active-token"]["refresh"]:"";
			$this->request->authorise($key,$refresh);
			$this->request->setBody(json_encode(array("lifetime"=>$lifetime)));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			
			$this->testers[$access] = array("key"=>array_key_exists("key",$responseBody)?$responseBody["key"]:"",
								"secret"=>array_key_exists("secret",$responseBody)?$responseBody["secret"]:"");
			
			if($authorise){
				$this->request->authorise(array_key_exists("key",$responseBody)?$responseBody["key"]:"",
								array_key_exists("secret",$responseBody)?$responseBody["secret"]:"");			
			}
			
			return $access;
		}
	}
?>