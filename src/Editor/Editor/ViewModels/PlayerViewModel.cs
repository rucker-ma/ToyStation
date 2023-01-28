using System;
using LibVLCSharp.Shared;

namespace Editor.ViewModels
{
    public class PlayerViewModel : ViewModelBase,IDisposable
    {
        private readonly LibVLC _libVlc = new LibVLC();
        
        public MediaPlayer MediaPlayer { get; } 
        public PlayerViewModel() {
            MediaPlayer = new MediaPlayer(_libVlc);
        }
        public void Play()
        {
           
            //using var media = new Media(_libVlc, new Uri("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"));
            using var media = new Media(_libVlc, "D:\\project\\ToyStation\\resource/test-video.mp4");
            
            MediaPlayer.Play(media);
        }

        public void Stop()
        {
            MediaPlayer.Stop();
        }
        public void Dispose()
        {
            MediaPlayer?.Dispose();
            _libVlc?.Dispose();

        }
    }
}