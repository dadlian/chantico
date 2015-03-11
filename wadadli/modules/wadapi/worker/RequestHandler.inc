<?php
	class RequestHandler extends Worker{
		private static $request;
		private static $requestURI;
		private static $endpoint;
		private static $uriArguments;
		private static $queryParameters;
		private static $acceptables;
		
		public static function getHost(){
			if(!self::$request){
				self::initialise();
			}
			
			return self::$request->getHost();
		}
		
		public static function getRequestURI(){
			if(!self::$requestURI){
				if(!self::$request){
					self::initialise();
				}
				
				//Extract the desired endpoint based on the project URL Root
				$regex = "http:\/\/(www\.)?".preg_replace("/\//","\/",SettingsManager::getSetting("install","urlroot"));
				$fullPath = urldecode("http://".RequestHandler::getHost().self::$request->getEndpoint());
				$requestURI = implode("/",array_compress(preg_split("/\//", preg_replace("/$regex/","",$fullPath))));
				self::$requestURI = implode("/",array_compress(preg_split("/\//",$requestURI)));
			}
			
			return self::$requestURI;
		}
		
		public static function getEndpoint(){
			if(!self::$endpoint){
				if(!self::$request){
					self::initialise();
				}
				
				$endpointsGateway = new EndpointsGateway();
				self::$endpoint = $endpointsGateway->findUnique('path',self::getRequestURI());
			}
			
			return self::$endpoint;
		}
		
		public static function getURIArguments(){
			if(!self::$uriArguments){
				if(!self::$request){
					self::initialise();
				}
				
				//Extract arguments from endpoint
				$endpoint = new URLPattern(self::getRequestURI());
				$endpointArguments = $endpoint->extractArguments(self::getEndpoint()->getPath());

				$i = 0;
				$uriArguments = array();
				foreach(self::getEndpoint()->getParameters() as $parameter){
					$uriArguments[(string)$parameter] = urldecode($endpointArguments[$i]);
					$i++;
				}
				
				self::$uriArguments = $uriArguments;
			}
			
			return self::$uriArguments;
		}
		
		public static function getQueryParameters(){
			if(!self::$queryParameters){
				if(!self::$request){
					self::initialise();
				}
				
				$queryParameters = array();
				$queryParts = preg_split("/&/",$_SERVER['QUERY_STRING']);
				foreach($queryParts as $queryPart){
					$keyValue = preg_split("/=/",$queryPart);
					if(sizeof($keyValue) > 1){
						$queryParameters[$keyValue[0]] = $keyValue[1];
					}
				}
				self::$queryParameters = $queryParameters;
			}
			
			return self::$queryParameters;
		}
		
		public static function getQueryParameter($parameter){
			if(!self::$queryParameters){
				self::getQueryParameters();
			}
			
			return array_key_exists($parameter,self::$queryParameters)?self::$queryParameters[$parameter]:"";
		}
		
		public static function getMethod(){
			if(!self::$request){
				self::initialise();
			}
			
			return self::$request->getMethod();
		}
		
		public static function getArgument($key){
			if(!self::$request){
				self::initialise();
			}
			
			return self::$request->viewFromArguments($key);
		}
		
		public static function getAuthorisation(){
			if(!self::$request){
				self::initialise();
			}
			
			$base64 = "(?:[A-Za-z0-9\+\/]{4})*(?:[A-Za-z0-9\+\/]{2}==|[A-Za-z0-9\+\/]{3}=)?";
			if(self::$request->viewFromHeaders("Authorization") && preg_match("/^Basic $base64$/",self::$request->viewFromHeaders("Authorization"))){
				$authorisation = array("key"=>"","secret"=>"");
				
				$authorisationParts = preg_split("/ /",self::$request->viewFromHeaders("Authorization"));
				$authorisationTokens = preg_split("/:/",base64_decode($authorisationParts[1]));
				
				if(sizeof($authorisationTokens) > 0){
					$authorisation["key"] = $authorisationTokens[0];
				}
				
				if(sizeof($authorisationTokens) > 1){
					$authorisation["secret"] = $authorisationTokens[1];
				}
				
				return $authorisation;
			}else{
				return false;
			}
		}
		
		public static function getAuthenticatedToken(){
			$authorisation = self::getAuthorisation();
			if(!$authorisation){
				return new APIToken();
			}
			
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($authorisation["key"]));
			return $sqlGateway->findUnique("APIToken",$searcher);
		}
		
		public static function checksConsistency(){
			return self::$request->viewFromHeaders("X-If-Unmodified-Since") || self::$request->viewFromHeaders("X-If-Match");
		}
		
		public static function getConsistencyTags(){
			$modifiedDate = date("U",strtotime(self::$request->viewFromHeaders("X-If-Unmodified-Since")));
			return array("ifUnmodifiedSince"=>$modifiedDate,"ifMatch"=>self::$request->viewFromHeaders("X-If-Match"));
		}
		
		public static function getSuggestedURI(){
			return self::$request->viewFromHeaders("Slug")?self::$request->viewFromHeaders("Slug"):"";
		}
		
		public static function isTest(){
			if(!self::$request){
				self::initialise();
			}
			
			return self::$request->viewFromHeaders("X-Test-Run") && self::$request->viewFromHeaders("X-Test-Run") === "True";
		}
		
		public static function getContent(){
			if(!self::$request){
				self::initialise();
			}
			
			$acceptedTypes = array_keys(self::$acceptables["Content-Type"]);
			if($acceptedTypes[0] == "application/json"){
				$content = json_decode(self::$request->getBody(),true);
				if(json_last_error() == JSON_ERROR_NONE){
					return $content?$content:array();
				}else{
					ResponseHandler::bad("Request arguments most be supplied using valid JSON.");
				}
			}else{
				return array("content"=>self::$request->getBody());
			}
		}
		
		private static function initialise(){
			//Parse query string for argument keys and values
			$arguments = array();
			if($_SERVER['QUERY_STRING']){
				foreach(preg_split("/\&/",urldecode($_SERVER['QUERY_STRING'])) as $queryPart){
					$argumentParts = preg_split("/=/",$queryPart);
					$arguments[$argumentParts[0]] = $argumentParts[1];
				}
			}
			
			//Parse Accept Headers
			self::$acceptables = array();
			$unsupportedFormats = self::parseAcceptHeader("Accept","formats","Content-Type");
			$unsupportedCharsets = self::parseAcceptHeader("Accept-Charset","charsets","Content-Charset");
			$unsupportedLanguages = self::parseAcceptHeader("Accept-Language","languages","Content-Language");
			
			if($unsupportedFormats || $unsupportedCharsets || $unsupportedLanguages){
				$unsupported = array();
				
				if($unsupportedFormats){
					$unsupported[] = "formats: ".implode(", ",$unsupportedFormats);
				}
				
				if($unsupportedCharsets){
					$unsupported[] = "charsets: ".implode(", ",$unsupportedCharsets);
				}
				
				if($unsupportedLanguages){
					$unsupported[] = "languages: ".implode(", ",$unsupportedLanguages);
				}
				
				ResponseHandler::unacceptable("The requested end-point does not support ".implode(", ",$unsupported).".");
			}
		
			//Set request body to raw input value
			$body = file_get_contents("php://input");
			
			
			ResponseHandler::changeContentType(self::$acceptables['Content-Type'][0]);
			ResponseHandler::changeContentCharset(self::$acceptables['Content-Charset'][0]);
			ResponseHandler::changeContentLanguage(self::$acceptables['Content-Language'][0]);
			self::$request = new WadapiRequest($_SERVER['HTTP_HOST'],preg_replace("/\?".preg_replace("/\//","\/",preg_quote($_SERVER['QUERY_STRING']))."/","",$_SERVER['REQUEST_URI']),
								$_SERVER['REQUEST_METHOD'],$arguments,getallheaders(),
								self::$acceptables['Content-Type'][0],$body);
		}
		
		private static function parseAcceptHeader($header,$setting,$type){
			$supportedValues = explode(",",SettingsManager::getSetting("api",$setting));
			$defaultValue = $supportedValues?$supportedValues[0]:"";
			$headers = getallheaders();
			$acceptHeader = array_key_exists($header,$headers)?$headers[$header]:$defaultValue;
			if(!preg_match("/([\w\/\+]+(;q\=[0-9\.]+)?,?)+/",$acceptHeader)){
				ResponseHandler::bad("The $header Header was malformed.");
			}
			
			$acceptedValues = array();
			foreach(explode(",",$acceptHeader) as $value){
				$valueParts = explode(";",$value);
				
				$priority = 1.0;
				if(sizeof($valueParts) > 1){
					$priorityParts = explode("=",$valueParts[1]);
					$priority = floatval($priorityParts[1]);
				}
				
				$acceptedValues[strtolower($valueParts[0])] = $priority;
			}
			
			arsort($acceptedValues);
			self::$acceptables[$type] = array_keys($acceptedValues);
			
			return array_diff(array_keys($acceptedValues),$supportedValues);
		}
	}
?>