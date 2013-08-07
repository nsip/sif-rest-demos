/*
 * BasicSIFRSConnection.java
 * Created: 25/07/2013
 *
 * Copyright 2013 Systemic Pty Ltd
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied.
 * See the License for the specific language governing permissions and limitations under the License.
 */

package systemic.sif3.infra.demo.basic;

import java.net.URI;
import java.nio.charset.Charset;

import javax.ws.rs.core.HttpHeaders;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response.Status;
import javax.ws.rs.core.UriBuilder;

import com.sun.jersey.api.client.Client;
import com.sun.jersey.api.client.ClientResponse;
import com.sun.jersey.api.client.WebResource;
import com.sun.jersey.api.client.WebResource.Builder;
import com.sun.jersey.api.client.config.ClientConfig;
import com.sun.jersey.api.client.config.DefaultClientConfig;
import com.sun.jersey.api.client.filter.LoggingFilter;
import com.sun.jersey.core.util.Base64;

/**
 * @author Joerg Huber
 *
 */
public class BasicSIFRSConnection
{
	//public static final String AUTH_TOKEN = "Basic MmU1ZGQzY2EyODJmYzhkZGIzZDA4ZGNhY2M0MDdlOGE6MDJiY2VjZjdlYjA5N2U3NzgzMTk1ZjBlZDJhNmEwNmIK";
	//public static final String AUTH_TOKEN = "Basic bmV3Omd1ZXN0";
	
	private static final String CONSUMER_SECRET = "guest";

	public static final String ENV_XML =
		"<environment>"+
		"  <solutionId>testSolution</solutionId>"+
		"  <authenticationMethod>Basic</authenticationMethod>"+
		"  <userToken></userToken>"+
		"  <applicationInfo>"+
		"    <applicationKey>TEST_CODE</applicationKey>"+
		"    <consumerName>SAMPLE AU Subscriber</consumerName>"+
		"    <supportedInfrastructureVersion>3.0</supportedInfrastructureVersion>"+
		"    <supportedDataModel>SIF-AU</supportedDataModel>"+
		"    <supportedDataModelVersion>3.0</supportedDataModelVersion>"+
		"    <transport>REST</transport>"+
		"    <applicationProduct>"+
		"      <vendorName>Systemic Pty Ltd</vendorName>"+
		"      <productName>Test Driver</productName>"+
		"      <productVersion>0.1</productVersion>"+
		"    </applicationProduct>"+
		"  </applicationInfo>"+
		"</environment>";
	
	// Once an Environment is created, copy the Environment ID and the Session Token into the next two variables.
	// These two values are taken from the POST environments response.
	private static final String ENV_GUID = "e1fd6cd6-898c-4854-ba66-f323c56f3414";
	private static final String SESSION_TOKEN = "9703043f0b00b1401de10ae31cc934ee";
	
	/*---------------------*/
	/*-- Utility Methods --*/
	/*---------------------*/
	private static URI getBaseURI()
	{
		return UriBuilder.fromUri("http://rest3api.sifassociation.org/api/environments").build();
	}
	
	private static String getAuthToken(String userToken)
	{
		String authToken = "Basic "+new String(Base64.encode(userToken+":"+CONSUMER_SECRET), Charset.forName("ASCII"));
		System.out.println("Authorisation Token for user token = '"+userToken+"': '"+authToken+"'");
		return authToken;
	}
	
	private static void printClientResponse(ClientResponse response)
	{
		System.out.println("Status: " + response.getStatus());
		System.out.println("Client Response Status Code: " + response.getClientResponseStatus().getStatusCode());
		System.out.println("Client Response Status Message: " + response.getClientResponseStatus().getReasonPhrase());
		System.out.println("Media Type: " + response.getType());
		System.out.println("Content Length: " + response.getLength());
		System.out.println("Has Entity: " + response.hasEntity());
		if (response.hasEntity() && (response.getClientResponseStatus().getStatusCode()!=Status.NO_CONTENT.getStatusCode()))
		{
			String msg = response.getEntity(String.class);
			System.out.println("Entity as String: " + msg);
		}
		System.out.println("Headers: " + response.getHeaders());
	}
	
	private static Builder setRequestHeader(WebResource service, String authToken)
	{
		System.out.println("Auth Token to use: "+authToken);
		return service.header(HttpHeaders.AUTHORIZATION, authToken); 
	}

	/*----------------------------------------*/
	/*-- Create, Read & Remove Environments --*/
	/*----------------------------------------*/
	private static void createEnvironment(WebResource service)
	{
		service = service.path("environment");
		String authTokenStep1 = getAuthToken("new");		
		ClientResponse response = setRequestHeader(service, authTokenStep1).type(MediaType.APPLICATION_XML).accept(MediaType.APPLICATION_XML).post(ClientResponse.class, ENV_XML);
		printClientResponse(response);
		
		// At this point you should copy the environmentID and Session Token from the Environment XML in the
		// response to the constants defined at the top of this class before using the getEnvironment() or 
		// removeEnvironment() method.
	}
	
	private static void getEnvironment(WebResource service)
	{
		service = service.path(ENV_GUID);
		String authTokenStep2 = getAuthToken(SESSION_TOKEN);
		ClientResponse response = setRequestHeader(service, authTokenStep2).accept(MediaType.APPLICATION_XML).get(ClientResponse.class);
		printClientResponse(response);
	}
	
	private static void removeEnvironment(WebResource service)
	{
	    service = service.path(ENV_GUID);
		String authTokenStep2 = getAuthToken(SESSION_TOKEN);
	    ClientResponse response = setRequestHeader(service, authTokenStep2).accept(MediaType.APPLICATION_XML).delete(ClientResponse.class);
	    printClientResponse(response);
	}
	
	public static void main(String[] args)
	{
		ClientConfig config = new DefaultClientConfig();
		
		// Create the Client Service
		Client client = Client.create(config);
		
		// Add logging of header information. Print info to System out.
		client.addFilter(new LoggingFilter(System.out));
		
		// Retrieve connector to resource
		WebResource service = client.resource(getBaseURI());

		// Un-comment each action as you like to use it. Remember you need to generate the environment first
		// before calling the two other methods.
		createEnvironment(service);
		//getEnvironment(service);
		//removeEnvironment(service);
	}

}
