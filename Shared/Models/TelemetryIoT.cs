namespace Shared.Models
{
    public class TelemetryIoT
    {
        public string? DeviceId { get; set; }
        public float Humidity { get; set; }
        public float Temperature { get; set; }
        public DateTime Date { get; set; }
        public string DateString
        {
            get
            {
                return Date.ToString("MM-dd-yyyy HH:mm:ss");
            }
        }
    }
}
