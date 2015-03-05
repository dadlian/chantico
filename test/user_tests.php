<?php	
	class UserTests extends APITestCase{
		function testSuccessfulUserCollectionManipulation(){
			/**** Test new user creation ****/
			$username = "tester";
			$password = md5("tester");
			$role = "tester";
			
			$this->request->setEndpoint("/users");
			$payload = json_encode(array("username"=>$username,"password"=>$password,"role"=>$role));
			$this->request->setBody($payload);
			$response = $this->request->post();
			
			//Test for Valid Status Line
			$this->assertEqual(201,$response->getStatusCode(),"Successful user creation returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("Created",$response->getReason(),"Successful user creation returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."/users/[0-9]{14}"))."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user creation responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user creation responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user creation responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Test for Resource Specific Headers
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Location"),"Successful user creation responds with the incorrect Location: ".$response->viewFromHeaders("Location"));
						
			//Ensure there are no unexpected Headers
			$this->assertEqual(8,sizeof($response->getHeaders()),"Successful user creation responds with superfluous headers (6 expected, ".sizeof($response->getHeaders())." given)");
			
			//Test new User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user creation does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully created user resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully created user resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully created user resource does not include a username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($username,$responseBody["username"],"A successfully created user resource includes the wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully created user resource does not include a role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($role,$responseBody["role"],"A successfully created user resource includes the wrong role: {$responseBody['role']}");
				}
				
				$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully created user resource does not include a tokens link.");
				if(array_key_exists("tokens",$responseBody)){
					$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully created user resource includes an invalid tokens link: {$responseBody['tokens']}");
				}
				
				$this->assertTrue(array_key_exists("options",$responseBody),"A successfully created user resource does not include an options array.");
				if(array_key_exists("options",$responseBody)){
					$this->assertEqual(array(),$responseBody["options"],"A successfully created user resource includes an invalid options array: {$responseBody['options']}");
				}
			}
			
			/**** Test new user collection retrieval ****/
			
			//Add sample users
			$username = array("tester1","tester2","tester3");
			$password = array(md5("tester"),md5("tester"),md5("tester"));
			$role = array("tester","tester","tester");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$this->request->setBody(json_encode(array("username"=>$username[0],"password"=>$password[0],"role"=>$role[0],"options"=>array())));
			$this->request->post()->viewFromHeaders("Location");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$this->request->setBody(json_encode(array("username"=>$username[1],"password"=>$password[1],"role"=>$role[1],"options"=>array())));
			$this->request->post()->viewFromHeaders("Location");
			
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$this->request->setBody(json_encode(array("username"=>$username[2],"password"=>$password[2],"role"=>$role[2],"options"=>array())));
			$this->request->post()->viewFromHeaders("Location");
			
			//Get full users collection
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving user collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved user collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved user collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved user collection does not include an array of users.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(4,sizeof($responseBody["entries"]),"A successfully retrieved user collection includes the incorrect number of users: ".sizeof($responseBody['entries']));
				
					//Check that contents of random user are valid
					$index = rand(0,2);
					$responseBody = $responseBody['entries'][$index+1];
					
					$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user resource does not include a self link.");
					if(array_key_exists("self",$responseBody)){
						$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/{[0-9]{14}[$index]}";
						$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user resource includes an invalid self link: {$responseBody['self']}");
					}
					
					$this->assertTrue(array_key_exists("username",$responseBody),"A successfully retrieved user resource does not include a username.");
					if(array_key_exists("username",$responseBody)){
						$this->assertEqual($username[$index],$responseBody["username"],"A successfully retrieved user resource includes the wrong username: {$responseBody['username']}");
					}
					
					$this->assertTrue(array_key_exists("role",$responseBody),"A successfully retrieved user resource does not include a role.");
					if(array_key_exists("role",$responseBody)){
						$this->assertEqual($role[$index],$responseBody["role"],"A successfully retrieved user resource includes the wrong role: {$responseBody['role']}");
					}
					
					$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully retrieved user resource does not include a tokens link.");
					if(array_key_exists("tokens",$responseBody)){
						$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}\/tokens";
						$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully retrieved user resource includes an invalid tokens link: {$responseBody['tokens']}");
					}
				
					$this->assertTrue(array_key_exists("options",$responseBody),"A successfully retrieved user resource does not include an options array.");
					if(array_key_exists("options",$responseBody)){
						$this->assertEqual(array(),$responseBody["options"],"A successfully retrieved user resource includes an invalid options array: {$responseBody['options']}");
					}
				}
			}
			
			//Get paged users collection
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$this->request->insertToArguments("page","2");
			$this->request->insertToArguments("records","1");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving paginated user collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved paginated user collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\?page=2&records=1";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved paginated user collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("prev",$responseBody),"A successfully retrieved paginated user collection does not include a link to the previous page.");
				if(array_key_exists("prev",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\?page=1&records=1";
					$this->assertPattern("/$userPattern/",$responseBody["prev"],"A successfully retrieved paginated user collection includes an invalid link to the previous page: {$responseBody['prev']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved paginated user collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved paginated user collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved paginated user collection does not include an array of users.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(1,sizeof($responseBody["entries"]),"A successfully retrieved paginated user collection includes the incorrect number of users: ".sizeof($responseBody['entries']));
				}
			}
		}
		
		function testUserCollectionManipulationHandlesUnsupportedRequests(){
			/**** Test unsupported methods ****/
			$this->request->setEndpoint("/users");
			
			$response = $this->request->put();
			$this->assertEqual(405,$response->getStatusCode(),"/users endpoint returns incorrect status code for unsupported method PUT: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users endpoint returns incorrect reason for unsupported method PUT: {$response->getReason()}");
			$this->validateErrorMessage($response,"/users does not support the PUT method.","Making an unsupported PUT request to the /users endpoint returns an invalid message");
			
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/users endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"/users does not support the DELETE method.","Making an unsupported DELETE request to the /users endpoint returns an invalid message");
		}
		
		function testUserCollectionManipulationHandlesMissingOrInvalidQueryParameters(){
			$this->request->setEndpoint("/users");
			
			/**** Test Invalid Query Parameters ****/
			$this->request->insertToArguments("page","page");
			$this->request->insertToArguments("records","records");
			$response = $this->request->get();
			$this->assertEqual(400,$response->getStatusCode(),"/users endpoint returns incorrect status code for invalid query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users endpoint returns incorrect reason for invalid query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: groups, page, records.","Supplying invalid query parameter values to the /users endpoint returns an invalid message");
		}
		
		function testUserCollectionManipulationHandlesMissingOrInvalidArguments(){
			$this->request->setEndpoint("/users");
			
			
			/**** Test Non-JSON Request Body ****/
			$this->request->setBody("Invalid JSON");
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/users endpoint returns incorrect status code for invalid JSON request body: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users endpoint returns incorrect reason for invalid JSON request body: {$response->getReason()}");
			$this->validateErrorMessage($response,"Request arguments most be supplied using valid JSON.","POSTing to the /users endpoint with a non-JSON body returns an invalid message");
			
			
			/**** Test Missing Required Arguments ****/
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/users endpoint returns incorrect status code for missing required arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users endpoint returns incorrect reason for missing required arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments are required, but have not been supplied: name.","Failing to supply required arguments to the /users endpoint returns an invalid message");
			
			
			/**** Test Invalid Arguments ****/
			$payload = json_encode(array("username"=>array("user"),"password"=>"not md5","role"=>false,"options"=>"invalid options"));
			$this->request->setBody($payload);
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/users endpoint returns incorrect status code for invalid arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users endpoint returns incorrect reason for invalid arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments have invalid values: username, password, role, options.","Supplying invalid argument values to the /users endpoint returns an invalid message");
			
			
			/**** Test Duplicate Argument Values ****/
			$this->request->setBody(json_encode(array("username"=>"orig_user","password"=>md5("test"),"role"=>"tester")));
			$this->request->post();
			
			//Make Duplicate Post
			$response = $this->request->post();
			$this->assertEqual(409,$response->getStatusCode(),"Creating a duplicate user resource returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Creating a duplicate user resource returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"An user identified by 'username' already exists.","Creating a duplicate user resource returns an invalid message");
			
			
			/**** Test Invalid Options Directives ****/
			$this->request->setBody(json_encode(array("username"=>array("username"),"password"=>md5("test"),"role"=>"tester","options"=>array("organisation"=>"http;//google.com"))));
			$response = $this->request->post();
			$this->assertEqual(409,$response->getStatusCode(),"Creating a user resource with invalid directives returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Creating a user resource with invalid directives returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The given options directives are invalid. No user could be created.","Creating a user resource with invalid directives returns an invalid message");
		}
		
		function testUserCollectionManipulationHandlesUnauthorisedRequests(){
			$this->request->setEndpoint("/users");

			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->post();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /users endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Unauthorized",$response->getReason(),"Unauthorised request to /users endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /users endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an Unauthorised request to the /users endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->request->authorise("baduser","badpass");
			$response = $this->request->post();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /users endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /users endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /users endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserCollectionManipulationNegotiatesContent(){
			$this->request->setEndpoint("/users");
			
			
			/**** Test that correct content types are returned by default ****/
			$this->request->setBody(json_encode(array("username"=>array("next_user0"),"password"=>md5("test"),"role"=>"tester")));
			$response = $this->request->post();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user creation request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user creation request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$this->request->setBody(json_encode(array("username"=>array("next_user1"),"password"=>md5("test"),"role"=>"tester")));
			$response = $this->request->post();
			
			$this->assertEqual(201,$response->getStatusCode(),"A successful user creation request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("Created",$response->getReason(),"A successful user creation request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user creation request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user creation request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("ru");
			$this->request->setBody(json_encode(array("username"=>array("next_user2"),"password"=>md5("test"),"role"=>"tester")));
			$response = $this->request->post();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /users endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /users endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support charsets: utf-8, languages: ru.","Requesting unsupported content types from the /users endpoint returns an incorrect message");
		}
		
		function testRetrievalOfUndefinedUserCollectionPages(){
			$this->request->setEndpoint("/users");
			
			/**** Test requesting a non-existant collection page ****/
			$this->request->insertToArguments("page","12");
			$this->request->insertToArguments("records","3");
			$response = $this->request->get();
			
			$this->assertEqual(404,$response->getStatusCode(),"Accessing non-existant pages of the /users endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing non-existant pages of the /users endpoint returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The specified page does not exist for the given records per page.","Accessing non-existant pages of the /users endpoint returns an invalid message");
		}
		
		
		function testSuccessfulUserManipulation(){
			//Create a new user resource
			$username = "tester3";
			$password = "tester";
			$role = "tester";
			
			$this->request->setEndpoint("/users");
			$payload = json_encode(array("username"=>$username,"password"=>md5($password),"role"=>$role));
			$this->request->setBody($payload);
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test User Resource is correctly retrieved ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot).$user)."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			$this->assertPattern($datePattern,$response->viewFromHeaders("Last-Modified"),"Successful user retrieval responds with a missing, malformed or incorrect Last-Modified Header: {$response->viewFromHeaders('Last-Modified')}");
			$this->assertPattern("/^[a-f0-9]{32}$/",$response->viewFromHeaders("ETag"),"Successful user retrieval responds with a missing or malformed ETag: {$response->viewFromHeaders('Last-Modified')}");
						
			//Ensure there are no unexpected Headers
			$this->assertEqual(9,sizeof($response->getHeaders()),"Successful user retrieval responds with superfluous headers (9 expected, ".sizeof($response->getHeaders())." given)");
						
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully retrieved user resource does not include a username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($username,$responseBody["username"],"A successfully retrieved user resource includes the wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully retrieved user resource does not include a role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($role,$responseBody["role"],"A successfully retrieved user resource includes the wrong role: {$responseBody['role']}");
				}
				
				$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully retrieved user resource does not include a tokens link.");
				if(array_key_exists("tokens",$responseBody)){
					$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully retrieved user resource includes an invalid tokens link: {$responseBody['tokens']}");
				}
				
				$this->assertTrue(array_key_exists("options",$responseBody),"A successfully retrieved user resource does not include an options array.");
				if(array_key_exists("options",$responseBody)){
					$this->assertEqual(array(),$responseBody["options"],"A successfully retrieved user resource includes an invalid options array: {$responseBody['options']}");
				}
			}
			
			
			/**** Test User Resource is correctly modified ****/
			$newUsername = "tester4"; $newPassword = "tester"; $newRole = "tester";
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("username"=>$newUsername,"password"=>md5($newPassword),"role"=>$newRole)));
			$this->request->authorise($username,$password);
			$response = $this->request->put();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user modification returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user modification returns the wrong reason: {$response->getReason()}");
						
			//Test for Modified User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user modification does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully modified user resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully modified user resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully modified user resource does not include a username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($newUsername,$responseBody["username"],"A successfully modified user resource includes the wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully modified user resource does not include a role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($newRole,$responseBody["role"],"A successfully modified user resource includes the wrong role: {$responseBody['role']}");
				}
				
				$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully modified user resource does not include a tokens link.");
				if(array_key_exists("tokens",$responseBody)){
					$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully modified user resource includes an invalid tokens link: {$responseBody['tokens']}");
				}
				
				$this->assertTrue(array_key_exists("options",$responseBody),"A successfully modified user resource does not include an options array.");
				if(array_key_exists("options",$responseBody)){
					$this->assertEqual(array(),$responseBody["options"],"A successfully modified user resource includes an invalid options array: {$responseBody['options']}");
				}
			}
			
			//Test that modifications were saved
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->get();
			
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(), true);
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully modified user resource does not save a new username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($newUsername,$responseBody["username"],"A successfully modified user resource saves a new wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully modified user resource does not save a new role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($newRole,$responseBody["role"],"A successfully modified user resource saves the wrong role: {$responseBody['role']}");
				}
			}
			
			
			/**** Test user resource is successfully removed ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$response = $this->request->delete();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user deletion returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user deletion returns the wrong reason: {$response->getReason()}");
						
			//Test for Removed User Response Content
			$this->validateErrorMessage($response,"User: $user, has been removed.","Removing a /user resource returns an incorrect message");
			
			//Test that user has been removed from the system
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->get();
			$this->assertNotEqual(200,$response->getStatusCode(),"A removed user resource is still available in the system after deletion.");
			
		}
		
		function testUserManipulationHandlesUnsupportedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test unsupported methods ****/
			$response = $this->request->post();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for unsupported method POST: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for unsupported method POST: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user does not support the POST method.","Making an unsupported POST request to the /users/{user_id} endpoint returns an invalid message");
		}
		
		function testUserManipulationHandlesMissingOrInvalidArguments(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_2"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Non-JSON Request Body ****/
			$this->request->setBody("Invalid JSON");
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for invalid JSON request body: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for invalid JSON request body: {$response->getReason()}");
			$this->validateErrorMessage($response,"Request arguments most be supplied using valid JSON.","PUTing to the /users/{user_id} endpoint with a non-JSON body returns an invalid message");
			
			
			/**** Test Missing Required Arguments ****/
			$this->request->setBody(json_encode(array()));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for missing required arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for missing required arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments are required, but have not been supplied: username, password, role.","Failing to supply required arguments to the /users/{user_id} endpoint returns an invalid message");
			
			
			/**** Test Invalid Arguments ****/
			$this->request->setBody(json_encode(array("username"=>"no spaces","password"=>$password,"role"=>true,"options"=>false)));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for invalid arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for invalid arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments have invalid values: name, active.","Supplying invalid argument values to the /users/{user_id} endpoint returns an invalid message");
			
			
			/**** Test Duplicate Argument Values ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/users");
			$username = "test_user_3"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$this->request->post();
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$username = "test_user_2"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			
			//Make Duplicate Put
			$response = $this->request->put();
			$this->assertEqual(409,$response->getStatusCode(),"Updating a user resource with a duplicate name returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Updating a user resource with a duplicate name returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"A user identified by 'test_user_2' already exists.","Updating a user resource with a duplicate name returns an invalid message");
			
						
			/**** Test Invalid Options Directives ****/
			$this->request->setBody(json_encode(array("username"=>array("username"),"password"=>md5("test"),"role"=>"tester","options"=>array("organisation"=>false))));
			$response = $this->request->put();
			$this->assertEqual(409,$response->getStatusCode(),"Creating a user resource with invalid directives returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Creating a user resource with invalid directives returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The given options directives are invalid. No user could be created.","Creating a user resource with invalid directives returns an invalid message");
		}
		
		function testUserManipulationHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_4"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->delete();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /users/{user_id} endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Unauthorized",$response->getReason(),"Unauthorised request to /users/{user_id} endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /users/{user_id} endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /users/{user_id} endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->initialiseRequest();
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /users/{user_id} endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /users/{user_id} endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /users/{user_id} endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserManipulationNegotiatesContent(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_5"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user retrieval request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user retrieval request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful user retrieval request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful user retrieval request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user retrieval request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user retrieval request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /users/{user_id} endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /users/{user_id} endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /users/{user_id} endpoint returns an incorrect message");
		}
		
		function testManipulationOfMissingUserResources(){
			/**** Test requesting a non-existant resource ****/
			$this->request->setEndpoint("/user/nonexistant");
			$this->request->assureConsistency("","");
			$response = $this->request->put();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /users/{user_id} endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /users/{user_id} endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested endpoint user/nonexistant does not exist on this server.","Accessing /users/{user_id} endpoint with non-existant user_id returns an invalid message");
		}
		
		function testUserManipulationConcurrencyChecks(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_6"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$validResponse = $this->request->get();
			
			
			/**** Test manipulating a resource without checking concurrency ****/
			$this->request->setBody(json_encode(array("username"=>"another_user","password"=>md5($password),"role"=>"tester")));
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Trying to manipulate /users/{user_id} endpoint without concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Trying to manipulate /users/{user_id} endpoint without concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"If-Unmodified-Since and If-Match Headers must be specified to modify this resource.","Manipulating /users/{user_id} without concurrency headers returns an invalid message");			
			
			
			/**** Test manipulating a resource with invalid concurrency tags ****/
			$this->request->assureConsistency($validResponse->viewFromHeaders("Last-Modified"),$validResponse->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("username"=>"another_user","password"=>md5($password),"role"=>"tester")));
			$this->request->put();
			
			//Repeat PUT with stale concurrency headers
			$response = $this->request->put();
			$this->assertEqual(412,$response->getStatusCode(),"Trying to manipulate /users/{user_id} endpoint with stale concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Precondition Failed",$response->getReason(),"Trying to manipulate /users/{user_id} endpoint with stale concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The wrong Modification Date and ETag values were given for this resource.","Manipulating /users/{user_id} with stale concurrency headers returns an invalid message");
		}
		
		
		function testSuccessfulUserTokensRetrieval(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_10"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			
			/**** Test User Tokens Resource is correctly retrieved ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user tokens retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user tokens retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot)."$user/tokens")."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user tokens retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user tokens retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user tokens retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Test for resource specific headers
			$expiresPattern = "/".gmdate("D, d M Y",time()+10)." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$this->assertPattern($expiresPattern,$response->viewFromHeaders("Expires"),"Successful Token Retrieval responds with the incorrect Expiry Date: ".$response->viewFromHeaders("Expires"));
			
			//Ensure there are no unexpected Headers
			$this->assertEqual(10,sizeof($response->getHeaders()),"Successful user tokens retrieval responds with superfluous headers (10 expected, ".sizeof($response->getHeaders())." given)");
						
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user tokens retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user's tokens resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user's tokens resource includes an invalid self link: {$responseBody['self']}");
				}
					
				$this->assertTrue(array_key_exists("key",$responseBody),"A successfully retrieved user's tokens resource does not include an MD5 key.");
				if(array_key_exists("key",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["key"],"A successfully retrieved user's tokens resource does not have an MD5 key: {$responseBody['key']}");
				}
				
				$this->assertTrue(array_key_exists("secret",$responseBody),"A successfully retrieved user's tokens resource does not include an MD5 secret.");
				if(array_key_exists("secret",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["secret"],"A successfully retrieved user's tokens resource does not have an MD5 secret: {$responseBody['secret']}");
				}
			}
		}
		
		function testUserTokensRetrievalHandlesUnsupportedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_11"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
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
		
		function testUserTokensRetrievalHandlesInvalidQueryParameters(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_12"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			
			/**** Test Invalid Query Parameters ****/
			$this->request->insertToArguments("page","page");
			$this->request->insertToArguments("records","records");
			$response = $this->request->get();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id}/tokens endpoint returns incorrect status code for invalid query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id}/tokens endpoint returns incorrect reason for invalid query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: page, records.","Supplying invalid query parameter values to the /users/{user_id}/tokens endpoint returns an invalid message");
		}
		
		function testUserTokensRetrievalHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_13"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
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
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /users/{user_id}/tokens endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /users/{user_id}/tokens endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /users/{user_id}/tokens endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserTokensRetrievalNegotiatesContent(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_14"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
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
			$this->request->setEndpoint("/user/nonexistant/tokens");
			$this->request->assureConsistency("","");
			$response = $this->request->put();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /users/{user_id}/tokens endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /users/{user_id}/tokens endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested endpoint user/nonexistant/tokens does not exist on this server.","Accessing /users/{user_id}/tokens endpoint with non-existant user_id returns an invalid message");
		}
		
		function testRetrievalOfUndefinedUserTokenPages(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "test_user_16"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test requesting a non-existant collection page ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->insertToArguments("page","1");
			$this->request->insertToArguments("records","5");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual(404,$response->getStatusCode(),"Accessing non-existant pages of the /users/{user_id}/tokens endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing non-existant pages of the /users/{user_id}/tokens endpoint returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The specified page does not exist for the given records per page.","Accessing non-existant pages of the /users/{user_id}/tokens endpoint returns an invalid message");
		}
		
		function testUserTokensAreRefreshed(){
			//Create a new user resource
			$this->request->setEndpoint("/users");
			$username = "refresh_user"; $password = "password";
			$this->request->setBody(json_encode(array("username"=>$username,"password"=>md5($password),"role"=>"tester")));
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
	}
?>