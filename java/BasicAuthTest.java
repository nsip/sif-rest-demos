import java.io.BufferedReader;
import java.io.InputStreamReader;
 
/*
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.AuthCache;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.protocol.ClientContext;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.auth.BasicScheme;
import org.apache.http.impl.client.BasicAuthCache;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.protocol.BasicHttpContext;
import org.junit.Test;
*/
 
 
 
public class BasicAuthTest {
 
      private String _body = "<environment><solutionId>testSolution</solutionId><authenticationMethod>Basic</authenticationMethod><applicationInfo><applicationKey>MIKER_TEST</applicationKey><consumerName>MIKER_CONSUMERNAME</consumerName><supportedInfrastructureVersion>3.0</supportedInfrastructureVersion><supportedDataModel>SIF-US</supportedDataModel><supportedDataModelVersion>3.0</supportedDataModelVersion><transport>REST</transport><applicationProduct><vendorName>X</vendorName><productName>X</productName><productVersion>X</productVersion></applicationProduct></applicationInfo></environment>";
     
      public void testCreateEnvironment(){
           
                  HttpHost targetHost = new HttpHost("rest3api.sifassociation.org", 80, "http");
                  //HttpHost targetHost = new HttpHost("localhost", 8084, "http");
 
              DefaultHttpClient httpclient = new DefaultHttpClient();
              try {
                  httpclient.getCredentialsProvider().setCredentials(
                          new AuthScope(targetHost.getHostName(), targetHost.getPort()),
                          new UsernamePasswordCredentials("new", "guest"));
 
                  AuthCache authCache = new BasicAuthCache();
                  BasicScheme basicAuth = new BasicScheme();
                  authCache.put(targetHost, basicAuth);
 
                  BasicHttpContext localcontext = new BasicHttpContext();
                  localcontext.setAttribute(ClientContext.AUTH_CACHE, authCache);
 
                  HttpPost httppost = new HttpPost("/api/environments/environment");
                  ContentType type = ContentType.create("application/xml", "utf-8");
                  StringEntity request = new StringEntity(_body, type);
                  httppost.setEntity(request);
                 
                  System.out.println("executing request: " + httppost.getRequestLine());
                  System.out.println("to target: " + targetHost);
 
                HttpResponse response = httpclient.execute(targetHost, httppost, localcontext);
                BufferedReader rd = new BufferedReader(new InputStreamReader(response.getEntity().getContent()));
                String line = "";
                while ((line = rd.readLine()) != null) {
                  System.out.println(line);
                }
 
 
              } catch(Exception e){
                  e.printStackTrace();
              }
              finally {
                  // When HttpClient instance is no longer needed,
                  // shut down the connection manager to ensure
                  // immediate deallocation of all system resources
                  httpclient.getConnectionManager().shutdown();
              }
            }
 

    public static void main(String[] args) {
        System.out.println("Hello, World");
    }

}


