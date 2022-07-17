using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;

using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using Shared.Models;
using Shared.Services;
using System;
using System.Threading.Tasks;

namespace EventHubTrigger
{
    public class ToBusisnessCentral
    { 
        [FunctionName("ToBusisnessCentral")]
        public async Task Run([IoTHubTrigger("iothub-ehub-ivansingle-20179072-XXXXXXX", Connection = "IoTHubEndpoints", ConsumerGroup = "$Default")] string message, ILogger log)
        {
            log.LogInformation($"IoT Hub trigger: message: {message}");
            
            ConfigurationsValues configValues = ReadEnviornmentVariable();
            if (configValues.Tenantid == null)
            {
                log.LogInformation("Please set the ConfigurationsValues.");
                return;
            }
            
            string MessageClean = message.Replace("\n", "").Replace("\r", "");
            var TelemetryIoT = JsonConvert.DeserializeObject<TelemetryIoT>(MessageClean);
            BCApiServices apiServices = new(configValues);
            var Result = await InsertTelemetry(configValues, TelemetryIoT, apiServices, log);
            log.LogInformation($"Finish: {Result.Message} / Time: {DateTime.Now}");
        }

        private static async Task<Response<object>> InsertTelemetry(
        ConfigurationsValues configValues,
        TelemetryIoT mTelemetryIoT,
        BCApiServices apiServices,
        ILogger log)
        {
            try
            {
                Response<object> Response = await apiServices.InsertInBusinessCentral(configValues.InsertTelemetry, mTelemetryIoT);
                var ResultBC = JsonConvert.DeserializeObject<Ouput>(Response.Message);
                var ResponseBC = JsonConvert.DeserializeObject<Response<string>>(ResultBC.value);
                
                if (Response.IsSuccess)
                {
                    return new Response<object>
                    {
                        IsSuccess = true,
                        Message = ResponseBC.Message
                    };
                }
                else
                {
                    return new Response<object>
                    {
                        IsSuccess = false,
                        Message = ResponseBC.Message
                    };
                }
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(ex.Message);
                log.LogInformation("Exception: " + ex.Message);
                
                return new Response<object>
                {
                    IsSuccess = false,
                    Message = "Exception: " + ex.Message
                };
            }
        }

        public static ConfigurationsValues ReadEnviornmentVariable()
        {
            ConfigurationsValues configValues = new()
            {
                Tenantid = Environment.GetEnvironmentVariable("Tenantid", EnvironmentVariableTarget.Process),
                ClientId = Environment.GetEnvironmentVariable("Clientid", EnvironmentVariableTarget.Process),
                ClientSecret = Environment.GetEnvironmentVariable("ClientSecret", EnvironmentVariableTarget.Process), 
                CompanyID = Environment.GetEnvironmentVariable("CompanyID", EnvironmentVariableTarget.Process),
                EnvironmentName = Environment.GetEnvironmentVariable("EnvironmentName", EnvironmentVariableTarget.Process)
            };

            return configValues;
        }
    }
}