using Newtonsoft.Json;

namespace Shared.Models
{
    public class Ouput
    {
        [JsonProperty("@odata.context")]
        public string OdataContext { get; set; }
        public string value { get; set; }
    }
}
