using Avalonia.Controls;
using LibVLCSharp.Avalonia;
using Editor.ViewModels;
namespace Editor.Views
{
    public partial class WebRtcView : VideoView
    {
        public WebRtcView()
        {
            InitializeComponent();
            var model = new WebRtcClientModel();
            DataContext= model;
            
        }
        public void OpenPlay()
        {
            var vm = DataContext as WebRtcClientModel;
            vm?.Init();
        }
    }
}
