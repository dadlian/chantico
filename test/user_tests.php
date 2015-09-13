<?php	
	class UserTests extends APITestCase{
		
		
		function testSuccessfulUserTokensRetrieval(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_10"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test User Tokens Resource is correctly retrieved ****/
			$this->request->setEndpoint("$user/tokens");
			$this->request->setBody("");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user tokens retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user tokens retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot)."/apis/14358019264956/$user/tokens")."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user tokens retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user tokens retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user tokens retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Ensure there are no unexpected Headers
			$this->assertEqual(9,sizeof($response->getHeaders()),"Successful user tokens retrieval responds with superfluous headers (10 expected, ".sizeof($response->getHeaders())." given)");
			exit;
			
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user tokens retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user's tokens resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user's tokens resource includes an invalid self link: {$responseBody['self']}");
				}
					
				$this->assertTrue(array_key_exists("key",$responseBody),"A successfully retrieved user's tokens resource does not include an MD5 key.");
				if(array_key_exists("key",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["key"],"A successfully retrieved user's tokens resource has an invalid MD5 key: {$responseBody['key']}");
				}
				
				$this->assertTrue(array_key_exists("secret",$responseBody),"A successfully retrieved user's tokens resource does not include an MD5 secret.");
				if(array_key_exists("secret",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["secret"],"A successfully retrieved user's tokens resource does has an invalid MD5 secret: {$responseBody['secret']}");
				}
				
				$this->assertTrue(array_key_exists("expiry",$responseBody),"A successfully retrieved user's tokens resource does not include an expiry timestamp.");
				if(array_key_exists("expiry",$responseBody)){
					$this->assertPattern("/^[0-9]{10}$/",$responseBody["expiry"],"A successfully retrieved user's tokens resource has an invalid expiry timestamp: {$responseBody['expiry']}");
				}
				
				$this->assertTrue(array_key_exists("renew",$responseBody),"A successfully retrieved user's tokens resource does not include a renewal link.");
				if(array_key_exists("renew",$responseBody)){
					$renewalPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."/apis/14358019264956/$user/renewals?warrant="))."[a-f0-9]{32}";
					$this->assertPattern("/^$renewalPattern$/",$responseBody["renew"],"A successfully retrieved user's tokens resource has an invalid renewal link: {$responseBody['renew']}");
				}
			}
		}
		
		function testUserTokensRetrievalHandlesUnsupportedRequests(){
			exit;
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_11"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test unsupported methods ****/
			$response = $this->request->put();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id}/tokens endpoint returns incorrect status code for unsupported method PUT: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id}/tokens endpoint returns incorrect reason for unsupported method PUT: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/tokens does not support the PUT method.","Making an unsupported PUT request to the /users/{user_id}/tokens endpoint returns an invalid message");
			
			$response = $this->request->post();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id}/tokens endpoint returns incorrect status code for unsupported method POST: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id}/tokens endpoint returns incorrect reason for unsupported method POST: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/tokens does not support the POST method.","Making an unsupported POST request to the /users/{user_id}/tokens endpoint returns an invalid message");
			
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id}/tokens endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id}/tokens endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/tokens does not support the DELETE method.","Making an unsupported DELETE request to the /users/{user_id}/tokens endpoint returns an invalid message");
		}
		
		function testUserTokensRetrievalHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_13"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->get();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /users/{user_id}/tokens endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Unauthorized",$response->getReason(),"Unauthorised request to /users/{user_id}/tokens endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /users/{user_id}/tokens endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /users/{user_id}/tokens endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise("Fake User","badpass");
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /users/{user_id}/tokens endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /users/{user_id}/tokens endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /users/{user_id}/tokens endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserTokensRetrievalNegotiatesContent(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_14"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user token retrieval request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user token retrieval request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful user tokens retrieval request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful user tokens retrieval request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user tokens retrieval request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user tokens retrieval request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /users/{user_id}/tokens endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /users/{user_id}/tokens endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /users/{user_id}/tokens endpoint returns an incorrect message");
		}
		
		function testeRetrievalOfMissingUserTokensResources(){
			/**** Test requesting a non-existant resource ****/
			$this->request->setEndpoint("/apis/14358019264956/user/nonexistant/tokens");
			$this->request->assureConsistency("","");
			$response = $this->request->put();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /users/{user_id}/tokens endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /users/{user_id}/tokens endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested endpoint user/nonexistant/tokens does not exist on this server.","Accessing /users/{user_id}/tokens endpoint with non-existant user_id returns an invalid message");
		}
		
		
		function testUserTokensAreRefreshed(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "refresh_user"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$key = ""; $secret = "";
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
					
				if(array_key_exists("key",$responseBody)){
					$key = $responseBody["key"];
				}
				
				if(array_key_exists("secret",$responseBody)){
					$secret = $responseBody["secret"];
				}
			}
			
			sleep(11);
			
			$response = $this->request->get();
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
					
				if(array_key_exists("key",$responseBody)){
					$this->assertNotEqual($key,$responseBody["key"],"The /users/{user_id}/tokens endpoint does not automatically refresh token key values.");
				}
				
				if(array_key_exists("secret",$responseBody)){
					$this->assertNotEqual($secret,$responseBody["secret"],"The /users/{user_id}/tokens endpoint does not automatically refresh token secret values.");
				}
			}
		}

		
		function testSuccessfulUserTokensRenewal(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1313"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$responseBody = json_decode($response->getBody(),true);
			$originalKey = $responseBody["key"];
			$originalSecret = $responseBody["secret"];
			$originalExpiry = $responseBody["expiry"];
			$originalRenew = $responseBody["renew"];
			
			//Test token renewal
			$this->initialiseRequest();
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			
			//Test for Valid Status Line
			$this->assertEqual(201,$response->getStatusCode(),"Successful user tokens renewal returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("Created",$response->getReason(),"Successful user tokens renewal returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot)."$user/renewals")."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user tokens renewal responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user tokens renewal responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user tokens renewal responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Ensure there are no unexpected Headers
			$this->assertEqual(8,sizeof($response->getHeaders()),"Successful user tokens renewal responds with superfluous headers (8 expected, ".sizeof($response->getHeaders())." given)");
			
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user tokens renewal does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("key",$responseBody),"A successfully created renewal resource does not include an MD5 key.");
				if(array_key_exists("key",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["key"],"A successfully created renewal resource has an invalid MD5 key: {$responseBody['key']}");
					$this->assertNotEqual($responseBody["key"], $originalKey,"A successfully created renewal resource does not re-issue the user token's key: {$responseBody['key']}");
				}
				
				$this->assertTrue(array_key_exists("secret",$responseBody),"A successfully created renewal resource does not include an MD5 secret.");
				if(array_key_exists("secret",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["secret"],"A successfully created renewal resource has an invalid MD5 secret: {$responseBody['secret']}");
					$this->assertNotEqual($responseBody["secret"], $originalSecret,"A successfully created renewal resource does not re-issue the user token's secret: {$responseBody['secret']}");
				}
				
				$this->assertTrue(array_key_exists("expiry",$responseBody),"A successfully created renewal resource does not include an expiry timestamp.");
				if(array_key_exists("expiry",$responseBody)){
					$this->assertPattern("/^[0-9]{10}$/",$responseBody["expiry"],"A successfully created renewal resource has an invalid expiry timestamp: {$responseBody['expiry']}");
					$this->assertTrue($responseBody["expiry"] > $originalExpiry,"A successfully created renewal resource does not extend the user token's validity: {$responseBody['expiry']}");
				}
				
				$this->assertTrue(array_key_exists("renew",$responseBody),"A successfully created renewal resource does not include a renewal link.");
				if(array_key_exists("renew",$responseBody)){
					$renewalPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?warrant=")."[a-f0-9]{32}");
					$this->assertPattern("/$renewalPattern/",$responseBody["renew"],"A successfully created renewal resource has an invalid renewal link: {$responseBody['renew']}");
					$this->assertNotEqual($responseBody["renew"], $originalRenew,"A successfully created renewal resource does not issue a new link for future renewals: {$responseBody['renew']}");
				}
			}
			
			/**** Test renewals collection retrieval ****/
			$renewedAt = array();
			
			//Renew token multiple times
			$this->initialiseRequest();
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			$renewedAt[] = date("c",$responseBody["expiry"]-10);
			
			$this->initialiseRequest();
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			$renewedAt[] = date("c",$responseBody["expiry"]-10);
			
			$this->initialiseRequest();
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			$renewedAt[] = date("c",$responseBody["expiry"]-10);
			
			
			//Get full renewals collection
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving token renewals collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved token renewals collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?page=1"));
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved token renewals collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved token renewals collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved token renewals collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved token renewals collection does not include an array of token renewals.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(4,sizeof($responseBody["entries"]),"A successfully retrieved token renewals collection includes the incorrect number of renewals: ".sizeof($responseBody['entries']));
				
					//Check that contents of random renewal are valid
					$index = rand(0,2);
					$responseBody = $responseBody['entries'][$index];
					rsort($renewedAt);
					
					$this->assertTrue(array_key_exists("renewed-at",$responseBody),"A successfully retrieved renewal resource does not include a renewed-at time.");
					if(array_key_exists("renewed-at",$responseBody)){
						$this->assertEqual($renewedAt[$index],$responseBody["renewed-at"],"A successfully retrieved renewal resource includes an invalid renewed-at time: {$responseBody['renewed-at']}");
					}
				}
			}
			
			//Get paged renewals collection
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$this->request->insertToArguments("page","2");
			$this->request->insertToArguments("records","2");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving paginated token renewals collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved paginated token renewals collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?page=2&records=2"));
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved paginated token renewals collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("prev",$responseBody),"A successfully retrieved paginated token renewals collection does not include a link to the previous page.");
				if(array_key_exists("prev",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?page=1&records=2"));
					$this->assertPattern("/$userPattern/",$responseBody["prev"],"A successfully retrieved paginated token renewals collection includes an invalid link to the previous page: {$responseBody['prev']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved paginated token renewals collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved paginated token renewals collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved paginated token renewals collection does not include an array of renewals.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(2,sizeof($responseBody["entries"]),"A successfully retrieved paginated token renewals collection includes the incorrect number of users: ".sizeof($responseBody['entries']));
				}
			}
		}
		
		function testUserTokensRenewalHandlesUnsupportedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1314"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$responseBody = json_decode($this->request->get()->getBody(),true);
			
			//Renew Token
			$this->initialiseRequest();
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			
			/**** Test unsupported methods ****/
			$response = $this->request->put();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id}/renewals endpoint returns incorrect status code for unsupported method PUT: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id}/renewals endpoint returns incorrect reason for unsupported method PUT: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/renewals does not support the PUT method.","Making an unsupported PUT request to the /users/{user_id}/renewals endpoint returns an invalid message");

			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id}/renewals endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id}/renewals endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/renewals does not support the DELETE method.","Making an unsupported DELETE request to the /users/{user_id}/renewals endpoint returns an invalid message");
		}
		
		function testUserTokensRenewalHandlesMissingOrInvalidQueryParameters(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1300"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$renewal = str_replace("http://{$this->apiRoot}","",$responseBody["renew"]);
			
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			
			/**** Test Invalid GET Query Parameters ****/
			$this->request->insertToArguments("page","page");
			$this->request->insertToArguments("records","records");
			$response = $this->request->get();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id}/renewals endpoint returns incorrect status code for invalid query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id}/renewals endpoint returns incorrect reason for invalid query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: page, records.","Supplying invalid query parameter values to the /users/{user_id}/renewals endpoint returns an invalid message");
			
			
			/**** Test Missing 'warrant' parameter ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id}/renewals endpoint returns incorrect status code for missing query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id}/renewals endpoint returns incorrect reason for missing query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters are required, but missing: warrant.","Failing to supply required query parameter values to the /users/{user_id}/renewals endpoint returns an invalid message");
		
			
			/**** Test Invalid 'warrant' parameter ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$this->request->insertToArguments("warrant",md5("badwarrant"));
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id}/renewals endpoint returns incorrect status code for invalid warrant: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id}/renewals endpoint returns incorrect reason for invalid warrant: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: warrant.","Supplying and invalid warrant to the /users/{user_id}/renewals endpoint returns an invalid message");
		
			/**** Test using a 'warrant' more than once ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($renewal);
			$response = $this->request->post();
			
			//Post again with same warrant
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id}/renewals endpoint returns incorrect status code for a previously used warrant: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id}/renewals endpoint returns incorrect reason for a previously used warrant: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: warrant.","Supplying a previously used warrant to the /users/{user_id}/renewals endpoint returns an invalid message");
		}
		
		function testUserTokensRenewalHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1315"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$renewal = str_replace("http://{$this->apiRoot}","",$responseBody["renew"]);
			
			//Renew Token
			$this->initialiseRequest();
			$this->request->setEndpoint($renewal);
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->get();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /users/{user_id}/renewals endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Unauthorized",$response->getReason(),"Unauthorised request to /users/{user_id}/renewals endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /users/{user_id}/renewals endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /users/{user_id}/renewals endpoint returns an invalid message");

			/**** Test Incorrectly Authorised Request ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($renewal);
			$this->request->authorise("Fake User","badpass");
			$response = $this->request->post();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /users/{user_id}/renewals endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /users/{user_id}/renewals endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /users/{user_id}/renewals endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserTokensRenewalNegotiatesContent(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1316"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			
			
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user token renewal request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user token renewal request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful user tokens renewal request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful user tokens renewal request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user tokens renewal request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user tokens renewal request for a supported content language was not honoured.");
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/renewals");
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /users/{user_id}/renewals endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /users/{user_id}/renewals endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /users/{user_id}/renewals endpoint returns an incorrect message");
		}
		
		function testRenewalOfMissingUserTokenResources(){
			/**** Test requesting a non-existant resource ****/
			$this->request->setEndpoint("/apis/14358019264956/user/nonexistant/renewals");
			$this->request->assureConsistency("","");
			$response = $this->request->get();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /users/{user_id}/renewals endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /users/{user_id}/renewals endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested endpoint user/nonexistant/renewals does not exist on this server.","Accessing /users/{user_id}/renewals endpoint with non-existant user_id returns an invalid message");
		
			
			/**** Test requesting a deleted (gone) resource ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1318"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			$this->request->delete();
			$response = $this->request->get();;
			$this->assertEqual(410,$response->getStatusCode(),"Accessing /users/{user_id}/renewals endpoint after the user has been removed returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Gone",$response->getReason(),"Accessing /users/{user_id}/renewals endpoint after the user has been removed returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested resource no longer exists.","Accessing /users/{user_id}/renewals endpoint after the user has been removed returns an invalid message");			
		}
	}
?>