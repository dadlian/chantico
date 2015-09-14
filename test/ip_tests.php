<?php
	class IPTests extends APITestCase{
		function testSuccessfulWhitelistManipulation(){
			/**** Test Whitelist is Successfully Retrieved ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful whitelist retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful whitelist retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."/apis/14358019264956/whitelist"))."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful whitelist retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful whitelist retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful whitelist retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			$this->assertPattern($datePattern,$response->viewFromHeaders("Last-Modified"),"Successful whitelist retrieval responds with a missing, malformed or incorrect Last-Modified Header: {$response->viewFromHeaders('Last-Modified')}");
			$this->assertPattern("/^[a-f0-9]{32}$/",$response->viewFromHeaders("ETag"),"Successful whitelist retrieval responds with a missing or malformed ETag: {$response->viewFromHeaders('Last-Modified')}");
						
			//Ensure there are no unexpected Headers
			$this->assertEqual(9,sizeof($response->getHeaders(false)),"Successful whitelist retrieval responds with superfluous headers (9 expected, ".sizeof($response->getHeaders())." given)");
						
			//Test Retrieved Group Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful whitelist retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved whitelist resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$this->assertPattern($locationPattern,$responseBody["self"],"A successfully retrieved whitelist resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved whitelist resource does not include a total.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(0,$responseBody["total"],"A successfully retrieved whitelist resource includes the wrong ip count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("ips",$responseBody),"A successfully retrieved whitelist resource does not include an ip list.");
				if(array_key_exists("ips",$responseBody)){
					$this->assertEqual(array(),$responseBody["ips"],"A successfully retrieved whitelist resource includes an invalid ip list: ".implode(",",$responseBody['ips']));
				}
			}
			
			
			/**** Test Whitelist are correctly modified ****/
			$newIPs = array("125.252.133.3","129.162.15.116","3.8.237.84","127.0.0.1");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$this->request->assureConsistency();
			$this->request->setBody(json_encode(array("ips"=>$newIPs)));
			$response = $this->request->put();
			exit;
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful whitelist modification returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful whitelist modification returns the wrong reason: {$response->getReason()}");

			//Test for Modified Group Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful whitelist modification does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("ips",$responseBody),"A successfully modified whitelist resource does not include an ip list.");
				if(array_key_exists("ips",$responseBody)){
					$this->assertEqual($newIPs,$responseBody["ips"],"A successfully modified whitelist resource includes the wrong ip list: ".implode(",",$responseBody["ips"]));
				}
			}
			
			//Test that modifications were saved
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(), true);
				
				$this->assertTrue(array_key_exists("ips",$responseBody),"A successfully modified whitelist resource does not save a new ip list.");
				if(array_key_exists("ips",$responseBody)){
					$this->assertEqual($newIPs,$responseBody["ips"],"A successfully modified whitelist resource saves the wrong ip list");
				}
			}
		}
		
		function testWhitelistManipulationHandlesUnsupportedRequests(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test unsupported methods ****/
			$this->request->setBody(json_encode(array("ips"=>array())));
			$response = $this->request->post();
			$this->assertEqual(405,$response->getStatusCode(),"/apis/14358019264956/whitelist endpoint returns incorrect status code for unsupported method POST: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/apis/14358019264956/whitelist endpoint returns incorrect reason for unsupported method POST: {$response->getReason()}");
			$this->validateErrorMessage($response,"/apis/14358019264956/whitelist does not support the POST method.","Making an unsupported POST request to the /whitelist endpoint returns an invalid message");
			
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/apis/14358019264956/whitelist endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/apis/14358019264956/whitelist endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"/apis/14358019264956/whitelist does not support the DELETE method.","Making an unsupported DELETE request to the /whitelist endpoint returns an invalid message");
		}
		
		function testWhitelistManipulationHandlesMissingOrInvalidArguments(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Non-JSON Request Body ****/
			$this->request->setBody("Invalid JSON");
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/14358019264956/whitelist endpoint returns incorrect status code for invalid JSON request body: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/14358019264956/whitelist endpoint returns incorrect reason for invalid JSON request body: {$response->getReason()}");
			$this->validateErrorMessage($response,"Request arguments must be supplied using valid JSON.","PUTing to the /whitelist endpoint with a non-JSON body returns an invalid message");
			
			
			/**** Test Missing Required Arguments ****/
			$this->request->setBody(json_encode(array()));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/14358019264956/whitelist endpoint returns incorrect status code for missing required arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/14358019264956/whitelist endpoint returns incorrect reason for missing required arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments are required, but have not been supplied: ips.","Failing to supply required arguments to the /whitelist endpoint returns an invalid message");
			
			
			/**** Test Invalid Arguments ****/
			$this->request->setBody(json_encode(array("ips"=>array("232323.2402.2402.12"))));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/14358019264956/whitelist endpoint returns incorrect status code for invalid arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/14358019264956/whitelist endpoint returns incorrect reason for invalid arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments have invalid values: ips.","Supplying invalid argument values to the /whitelist endpoint returns an invalid message");
						
						
			/**** Test duplicate URIs  in organisation list ****/
			$newIPs = array("ips"=>array("232.123.69.42","232.123.69.42"));
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode($newIPs));
			
			$response = $this->request->put();
			$this->assertEqual(409,$response->getStatusCode(),"Updating /whitelist endpoint with an ip list containing duplicates returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Updating /whitelist endpoint with an ip list containing duplicates returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The given ip list contains duplicates.","Updating /whitelist endpoint with an ip list containing duplicates returns an invalid message");
		}
		
		function testWhitelistManipulationHandlesUnauthorisedRequests(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->get();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /whitelist endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Authorization Required",$response->getReason(),"Unauthorised request to /whitelist endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /whitelists/{whitelist_slug}/ip endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /whitelist endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->request->authorise("baduser","badpass");
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /whitelist endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /whitelist endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /whitelist endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testWhitelistManipulationNegotiatesContent(){
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful whitelist retrieval request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful whitelist retrieval request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful whitelist retrieval request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful whitelist retrieval request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful whitelist retrieval request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful whitelist retrieval request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /whitelist endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /whitelist endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /whitelist endpoint returns an incorrect message");
		}
		
		function testWhitelistManipulationConcurrencyChecks(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$validResponse = $this->request->get();
			
			
			/**** Test manipulating a resource without checking concurrency ****/
			$this->request->setBody(json_encode(array("ips"=>array())));
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Trying to manipulate /whitelist endpoint without concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Trying to manipulate /whitelist endpoint without concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"If-Unmodified-Since and If-Match Headers must be specified to modify this resource.","Manipulating /whitelist without concurrency headers returns an invalid message");			
			
			
			/**** Test manipulating a resource with invalid concurrency tags ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>array("5.154.167.63","16.121.217.65","127.0.0.1"))));
			$this->request->put();
			
			//Repeat PUT with stale concurrency headers
			$response = $this->request->put();
			$this->assertEqual(412,$response->getStatusCode(),"Trying to manipulate /whitelist endpoint with stale concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Precondition Failed",$response->getReason(),"Trying to manipulate /whitelist endpoint with stale concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The wrong Modification Date and ETag values were given for this resource.","Manipulating /whitelist with stale concurrency headers returns an invalid message");
		}
		
		function testSuccessfulBlacklistManipulation(){
			/**** Test Blacklist is Successfully Retrieved ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful blacklist retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful blacklist retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."/apis/14358019264956/blacklist"))."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful blacklist retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful blacklist retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful blacklist retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			$this->assertPattern($datePattern,$response->viewFromHeaders("Last-Modified"),"Successful blacklist retrieval responds with a missing, malformed or incorrect Last-Modified Header: {$response->viewFromHeaders('Last-Modified')}");
			$this->assertPattern("/^[a-f0-9]{32}$/",$response->viewFromHeaders("ETag"),"Successful blacklist retrieval responds with a missing or malformed ETag: {$response->viewFromHeaders('Last-Modified')}");
						
			//Ensure there are no unexpected Headers
			$this->assertEqual(9,sizeof($response->getHeaders(false)),"Successful blacklist retrieval responds with superfluous headers (9 expected, ".sizeof($response->getHeaders())." given)");
						
			//Test Retrieved Group Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful blacklist retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved blacklist resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$this->assertPattern($locationPattern,$responseBody["self"],"A successfully retrieved blacklist resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved blacklist resource does not include a total.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(0,$responseBody["total"],"A successfully retrieved blacklist resource includes the wrong ip count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("ips",$responseBody),"A successfully retrieved blacklist resource does not include an ip list.");
				if(array_key_exists("ips",$responseBody)){
					$this->assertEqual(array(),$responseBody["ips"],"A successfully retrieved blacklist resource includes an invalid ip list: ".implode(",",$responseBody['ips']));
				}
			}
			
			
			/**** Test Blacklist are correctly modified ****/
			$newIPs = array("125.252.133.3","129.162.15.116","3.8.237.84");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>$newIPs)));
			$response = $this->request->put();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful blacklist modification returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful blacklist modification returns the wrong reason: {$response->getReason()}");
					
			//Test for Modified Group Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful blacklist modification does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("ips",$responseBody),"A successfully modified blacklist resource does not include an ip list.");
				if(array_key_exists("ips",$responseBody)){
					$this->assertEqual($newIPs,$responseBody["ips"],"A successfully modified blacklist resource includes the wrong ip list: ".implode(",",$responseBody["ips"]));
				}
			}
			
			//Test that modifications were saved
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(), true);
				
				$this->assertTrue(array_key_exists("ips",$responseBody),"A successfully modified blacklist resource does not save a new ip list.");
				if(array_key_exists("ips",$responseBody)){
					$this->assertEqual($newIPs,$responseBody["ips"],"A successfully modified blacklist resource saves the wrong ip list");
				}
			}
		}
		
		function testBlacklistManipulationHandlesUnsupportedRequests(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test unsupported methods ****/
			$this->request->setBody(json_encode(array("ips"=>array())));
			$response = $this->request->post();
			$this->assertEqual(405,$response->getStatusCode(),"/apis/14358019264956/blacklist endpoint returns incorrect status code for unsupported method POST: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/apis/14358019264956/blacklist endpoint returns incorrect reason for unsupported method POST: {$response->getReason()}");
			$this->validateErrorMessage($response,"/apis/14358019264956/blacklist does not support the POST method.","Making an unsupported POST request to the /blacklist endpoint returns an invalid message");
			
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/apis/14358019264956/blacklist endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/apis/14358019264956/blacklist endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"/apis/14358019264956/blacklist does not support the DELETE method.","Making an unsupported DELETE request to the /blacklist endpoint returns an invalid message");
		}
		
		function testBlacklistManipulationHandlesMissingOrInvalidArguments(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Non-JSON Request Body ****/
			$this->request->setBody("Invalid JSON");
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/14358019264956/blacklist endpoint returns incorrect status code for invalid JSON request body: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/14358019264956/blacklist endpoint returns incorrect reason for invalid JSON request body: {$response->getReason()}");
			$this->validateErrorMessage($response,"Request arguments must be supplied using valid JSON.","PUTing to the /blacklist endpoint with a non-JSON body returns an invalid message");
			
			
			/**** Test Missing Required Arguments ****/
			$this->request->setBody(json_encode(array()));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/14358019264956/blacklist endpoint returns incorrect status code for missing required arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/14358019264956/blacklist endpoint returns incorrect reason for missing required arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments are required, but have not been supplied: ips.","Failing to supply required arguments to the /blacklist endpoint returns an invalid message");
			
			
			/**** Test Invalid Arguments ****/
			$this->request->setBody(json_encode(array("ips"=>array("232323.2402.2402.12"))));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/14358019264956/blacklist endpoint returns incorrect status code for invalid arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/14358019264956/blacklist endpoint returns incorrect reason for invalid arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments have invalid values: ips.","Supplying invalid argument values to the /blacklist endpoint returns an invalid message");
						
						
			/**** Test duplicate URIs  in organisation list ****/
			$newIPs = array("ips"=>array("232.123.69.42","232.123.69.42"));
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode($newIPs));
			
			$response = $this->request->put();
			$this->assertEqual(409,$response->getStatusCode(),"Updating /blacklist endpoint with an ip list containing duplicates returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Updating /blacklist endpoint with an ip list containing duplicates returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The given ip list contains duplicates.","Updating /blacklist endpoint with an ip list containing duplicates returns an invalid message");
		}
		
		function testBlacklistManipulationHandlesUnauthorisedRequests(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->get();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /blacklist endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Authorization Required",$response->getReason(),"Unauthorised request to /blacklist endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /blacklists/{blacklist_slug}/ip endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /blacklist endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->request->authorise("baduser","badpass");
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /blacklist endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /blacklist endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /blacklist endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testBlacklistManipulationNegotiatesContent(){
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful blacklist retrieval request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful blacklist retrieval request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful blacklist retrieval request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful blacklist retrieval request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful blacklist retrieval request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful blacklist retrieval request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /blacklist endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /blacklist endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /blacklist endpoint returns an incorrect message");
		}
		
		function testBlacklistManipulationConcurrencyChecks(){
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$validResponse = $this->request->get();
			
			
			/**** Test manipulating a resource without checking concurrency ****/
			$this->request->setBody(json_encode(array("ips"=>array())));
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Trying to manipulate /blacklist endpoint without concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Trying to manipulate /blacklist endpoint without concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"If-Unmodified-Since and If-Match Headers must be specified to modify this resource.","Manipulating /blacklist without concurrency headers returns an invalid message");			
			
			
			/**** Test manipulating a resource with invalid concurrency tags ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>array("5.154.167.63","16.121.217.65"))));
			$this->request->put();
			
			//Repeat PUT with stale concurrency headers
			$response = $this->request->put();
			$this->assertEqual(412,$response->getStatusCode(),"Trying to manipulate /blacklist endpoint with stale concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Precondition Failed",$response->getReason(),"Trying to manipulate /blacklist endpoint with stale concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The wrong Modification Date and ETag values were given for this resource.","Manipulating /blacklist with stale concurrency headers returns an invalid message");			
		}
		
		function testWhitelistBlacklistMutualExclusivity(){
			$ips = array("241.107.72.174","205.63.67.58","181.101.104.69","116.229.51.252","232.131.248.249","127.0.0.1");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>$ips)));
			$this->request->put();
			
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$this->assertEqual($ips,$responseBody["ips"],"Initialising a Whitelist with sample IPs fails.");
			
			//Test adding IPs to the blacklist removes them from the whitelist
			$sample = array_unique(array($ips[rand(0,sizeof($ips)-2)],$ips[rand(0,sizeof($ips)-2)],$ips[rand(0,sizeof($ips)-2)]));
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>$sample)));
			$this->request->put();
			
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$this->assertEqual($sample,$responseBody["ips"],"Adding IPs to the /blacklist endpoint, does not work if they are already in the whitelist.");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$this->assertEqual(array_diff($ips,$sample),$responseBody["ips"],"Adding IPs to the /blacklist endpoint does not remove them from the whitelist.");
			
			//Test adding IPs to the whitelist removes them from the blacklist
			$ip = $sample[rand(0,sizeof($sample)-1)];
			$whitelist = array_merge(array_diff($ips,$sample),array($ip));
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>$whitelist)));
			$this->request->put();
			
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$this->assertEqual($whitelist,$responseBody["ips"],"Adding IPs to the /whitelist endpoint, does not work if they are already in the blacklist.");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$this->assertEqual(array_diff($sample,$whitelist),$responseBody["ips"],"Adding IPs to the /whitelist endpoint does not remove them from the blacklist.");
		}
		
		function testWhitelistRestrictsAccessCorrectly(){
			//Add own IP to whitelist
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/whitelist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>array("127.0.0.1"))));
			$this->request->put();
			
			//Make a test request
			$response = $this->request->get();
			$this->assertEqual(200,$response->getStatusCode(),"API access is incorrectly restricted for a whitelisted IP.");
		}
		
		function testBlacklistRestrictsAccessCorrectly(){
			//Add another IP to blacklist
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>array("57.72.82.164"))));
			$this->request->put();
			
			//Make a test request
			$response = $this->request->get();
			$this->assertEqual(200,$response->getStatusCode(),"API access for a non-blacklisted IP is incorrectly revoked when a blacklist is created.");
			
			//Add own IP to blacklist
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/blacklist");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("ips"=>array("127.0.0.1"))));
			$this->request->put();
			
			//Make a test request
			$response = $this->request->get();
			$this->assertEqual(403,$response->getStatusCode(),"API access is not revoked when an IP address is blacklisted.");
		}
	}
?>