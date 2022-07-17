using Microsoft.Identity.Client;
using Newtonsoft.Json;
using Shared.Models;
using System.Net;
using System.Net.Http.Headers;
using System.Text;


namespace Shared.Services
{
    public class BCApiServices
    {
        private static AuthenticationResult AuthResult = null;

        ConfigurationsValues mConfigurationsValues;

        public BCApiServices(ConfigurationsValues configurationsValues)
        {
            this.mConfigurationsValues = configurationsValues;
        }

        private async Task<AuthenticationResult> GetAccessToken(string TenantId)
        {
            Uri uri = new(mConfigurationsValues.Authority.Replace("{Tenantid}", TenantId));
            IConfidentialClientApplication app = ConfidentialClientApplicationBuilder.Create(mConfigurationsValues.ClientId)
                .WithClientSecret(mConfigurationsValues.ClientSecret)
                .WithAuthority(uri)
                .Build();
            string[] scopes = new string[] { @"https://api.businesscentral.dynamics.com/.default" };
            AuthenticationResult result = null;
            try
            {
                result = await app.AcquireTokenForClient(scopes).ExecuteAsync();
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("Token acquired");
                Console.ResetColor();
            }
            catch (MsalServiceException ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Error occurred while retrieving access token");
                Console.WriteLine($"{ex.ErrorCode} {ex.Message}");
                Console.ResetColor();
            }
            return result;
        }

        private static RequestBC CreateRequest(TelemetryIoT mTelemetryIoT)
        {
            string body = JsonConvert.SerializeObject(mTelemetryIoT);

            RequestBC requestBC = new()
            {
                jsontext = body
            };

            return requestBC;
        }

        public async Task<Response<object>> InsertInBusinessCentral(string BCUrl, TelemetryIoT mTelemetryIoT)
        {
            string result = string.Empty;
            if ((AuthResult == null) || (AuthResult.ExpiresOn < DateTime.Now))
            {
                AuthResult = await GetAccessToken(mConfigurationsValues.Tenantid);
            }

            using (HttpClient client = new())
            {
                client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", AuthResult.AccessToken);
                Uri uri = new(BCUrl);
                RequestBC requestBC = CreateRequest(mTelemetryIoT);
                string request = JsonConvert.SerializeObject(requestBC);
                StringContent content = new(request, Encoding.UTF8, "application/json");
                HttpResponseMessage response = await client.PostAsync(uri, content);
                result = await response.Content.ReadAsStringAsync();
                if ((response.StatusCode == HttpStatusCode.OK) || (response.StatusCode == HttpStatusCode.Created))
                {
                    return new Response<object>
                    {
                        IsSuccess = true,
                        Message = result
                    };
                }
                Console.ForegroundColor = ConsoleColor.Red;
                result = $"Call to Business Central API failed StatusCode: {response.StatusCode} ReasonPhrase: {response.ReasonPhrase} Result: {result}";
                Console.WriteLine(result);
                Console.ResetColor();
            }

            return new Response<object>
            {
                IsSuccess = false,
                Message = result
            };
        }
    }
}
