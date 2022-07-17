using Shared.Helper;
using System;
using System.IO;
//using Xamarin.Forms;

namespace Shared.Models
{
    public class Item : BaseViewModel
    {
        public string No { get; set; }
        public string? Description { get; set; }
        public decimal UnitPrice { get; set; }
        public decimal UnitCost { get; set; }
        public string? UnitOfMeasure { get; set; }

        public string Picture { get; set; }

        public byte[] _ImageBinary;
        //public byte[] PictureArray
        //{
        //    get
        //    {
        //        if (Picture == "NoPicture")
        //        {
        //            return null;
        //        }

        //        _ImageBinary = Convert.FromBase64String(Picture);

        //        PictureSource = ImageSource.FromStream(() => new MemoryStream(_ImageBinary));

        //        return _ImageBinary;
        //    }

        //    set => SetProperty(ref _ImageBinary, value);
        //}


        //public bool HasAttachementImage
        //{
        //    get
        //    {
        //        return _ImageBinary != null;
        //    }
        //}

        //public ImageSource PictureSource { get; set; }

    }
}

