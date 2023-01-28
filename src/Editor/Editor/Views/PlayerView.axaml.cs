using System;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Editor.ViewModels;
using LibVLCSharp.Avalonia;

namespace Editor.Views
{
    public partial class PlayerView : VideoView
    {
        
        public PlayerView()
        {
            InitializeComponent();
            var vm  = new PlayerViewModel();
            MediaPlayer = vm.MediaPlayer;
            DataContext = vm;
        }
        public void OpenPlay()
        {
            var vm = DataContext as PlayerViewModel;
            vm?.Play();
        }

        public void StopPlay()
        {
            var vm = DataContext as PlayerViewModel;
            vm?.Stop();
        }
    }
}
