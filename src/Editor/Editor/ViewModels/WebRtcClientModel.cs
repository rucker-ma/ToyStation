using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Linq;
using System.Net;
using System.IO;
using LibVLCSharp.Shared;
using SIPSorcery.Net;
using WebSocketSharp;
using WebSocketSharp.Net.WebSockets;
using WebSocketSharp.Server;


using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Serilog;
using Serilog.Extensions.Logging;
using Newtonsoft.Json;

namespace Editor.ViewModels
{
    
    public class WebRtcClient : WebSocketBehavior
    {
        public RTCPeerConnection pc;

        public event Func<WebSocketContext, Task<RTCPeerConnection>> WebSocketOpened;
        public event Func<WebSocketContext, RTCPeerConnection, string, Task> OnMessageReceived;

        public WebRtcClient()
        { }

        protected override void OnMessage(MessageEventArgs e)
        {
            OnMessageReceived(this.Context, pc, e.Data);
        }

        protected override async void OnOpen()
        {
            base.OnOpen();
            pc = await WebSocketOpened(this.Context);
        }
    }

    public class SignalingMessage
    {
        public string type { get; set;}
        public string? payload { get; set; }
    }
    
    public class WebRtcClientModel : ViewModelBase
    {
        private static Microsoft.Extensions.Logging.ILogger logger = NullLogger.Instance;
        
        private const int WEBSOCKET_PORT = 8081;
        private const int FFPLAY_DEFAULT_AUDIO_PORT = 5016;
        private const int FFPLAY_DEFAULT_VIDEO_PORT = 5018;
        private const string FFPLAY_DEFAULT_SDP_PATH = "ffplay.sdp";
        private static LibVLC _libVlc;
        private static MediaPlayer _mediaPlayer;
        private static RTCPeerConnection _activePeerConnection;

        private static WebSocket _websocket;


        private WebRtcClient _client;
        /// <summary>
        /// To filter the audio or video codecs when the initial offer is from the remote party
        /// add the desired codecs to these two lists. Leave empty to accept all codecs.
        /// 
        /// Note: During testing ffplay seemed to have problems if the SDP input file had multiple 
        /// codecs. It was observed to select the wrong codec for the RTP header payload ID it was 
        /// receiving. It may be that ffplay decides it can choose it's favorite codec and the remote
        /// party will honor that. The simple fix is to filter to a single audio and video codec.
        ///
        /// Set the codecs sent when the offer is made to the remote peer. Note that no encoding/decoding is
        /// done by this program. ffplay will need to support the selected codec.
        /// </summary>
        private static List<SDPAudioVideoMediaFormat> AudioOfferFormats = new List<SDPAudioVideoMediaFormat> {
            new SDPAudioVideoMediaFormat(SDPMediaTypesEnum.audio, 111, "OPUS", 48000, 2, "minptime=10;useinbandfec=1")
        };

        private static List<SDPAudioVideoMediaFormat> VideoOfferFormats = new List<SDPAudioVideoMediaFormat> {
            new SDPAudioVideoMediaFormat(SDPMediaTypesEnum.video, 100, "VP8", 90000)
        };


        public void Init()
        {
            logger = AddConsoleLogger();
            Console.WriteLine("Starting web socket server...");

            _websocket = new WebSocket("ws://127.0.0.1:9002");
            _websocket.OnOpen += (sender, e) =>
            {
                Console.WriteLine("Open Websocket");

            };
            _websocket.OnMessage += (sender, e) =>
            {

                var message = JsonConvert.DeserializeObject<SignalingMessage>(e.Data);
                if(message.type == "welcome")
                {
                    _client = new WebRtcClient();
                    _client.pc = CreatePC();

                    SignalingMessage send_msg= new SignalingMessage();
                    send_msg.type = "join";

                    string send_str = JsonConvert.SerializeObject(send_msg);
                    _websocket.Send(send_str);
                }
                if(message.type == "offer")
                {
                    if (message.payload!=null)
                    {
                        SetOffer(_client.pc, message.payload);
                        string answer = _client.pc.localDescription.sdp.ToString();
                        SignalingMessage send_msg = new SignalingMessage() ;
                        send_msg.type = "answer";
                        send_msg.payload = answer;
                        _websocket.Send(JsonConvert.SerializeObject(send_msg));

                    }
                }
                if(message.type == "candidate")
                {
                    if (message.payload!=null)
                    {
                        var candInit = Newtonsoft.Json.JsonConvert.DeserializeObject<RTCIceCandidateInit>(message.payload);
                        _client.pc.addIceCandidate(candInit);
                    }
                }
            };
            _websocket.Connect();
            
            //_webSocketServer = new WebSocketServer(IPAddress.Any, WEBSOCKET_PORT);
            //_webSocketServer.AddWebSocketService<WebRtcClient>("/", (client) =>
            //{
            //    client.WebSocketOpened += SendOffer;
            //    client.OnMessageReceived += WebSocketMessageReceived;
            //});
            //_webSocketServer.Start();

        }
        private static async void SetOffer(RTCPeerConnection pc,string message)
        {
            await WebSocketMessageReceived(pc, message);
        }
        private static async Task<RTCPeerConnection> SendOffer(WebSocketContext context)
        {
            var pc = CreatePC();
            MediaStreamTrack audioTrack = new MediaStreamTrack(SDPMediaTypesEnum.audio, false, AudioOfferFormats, MediaStreamStatusEnum.RecvOnly);
            pc.addTrack(audioTrack);
            MediaStreamTrack videoTrack = new MediaStreamTrack(SDPMediaTypesEnum.video, false, VideoOfferFormats, MediaStreamStatusEnum.RecvOnly);
            pc.addTrack(videoTrack);

            var offerInit = pc.createOffer(null);
            await pc.setLocalDescription(offerInit);
            context.WebSocket.Send(offerInit.sdp);
            return pc;
        }
        private static RTCPeerConnection CreatePC()
        {
           var pc = new RTCPeerConnection();
           pc.GetRtpChannel().OnStunMessageReceived += (msg, ep, isRelay) =>
           {
               bool hasUseCandidate = msg.Attributes.Any(x => x.AttributeType == STUNAttributeTypesEnum.UseCandidate);
               Console.WriteLine($"STUN {msg.Header.MessageType} received from {ep}, use candidate {hasUseCandidate}.");
            
           };
           pc.onicecandidateerror += (candidate, error) => logger.LogWarning($"Error adding remote ICE candidate. {error} {candidate}");
           pc.oniceconnectionstatechange += (state) => logger.LogDebug($"ICE connection state change to {state}.");
           // pc.OnReceiveReport += (type, rtcp) => logger.LogDebug($"RTCP {type} report received.");
            pc.OnRtcpBye += (reason) => logger.LogDebug($"RTCP BYE receive, reason: {(string.IsNullOrWhiteSpace(reason) ? "<none>" : reason)}.");
            pc.onicecandidate += (candidate) =>
            {
                if (pc.signalingState == RTCSignalingState.have_local_offer ||
                    pc.signalingState == RTCSignalingState.have_remote_offer)
                {
                    Console.WriteLine(candidate.ToString() );

                    //context.WebSocket.Send($"candidate:{candidate}");
                }
            };
            pc.onconnectionstatechange += (state) =>
            {
               if (state == RTCPeerConnectionState.connected)
               {
                   var rtpSession =
                       CreateRtpSession(pc.AudioLocalTrack?.Capabilities, pc.VideoLocalTrack?.Capabilities);
                   
                   Core.Initialize();
                   _libVlc = new LibVLC();
                   
                   _mediaPlayer = new MediaPlayer(_libVlc);
                   
                   var sdpFullPath = Path.Combine(Directory.GetParent(typeof(Program).Assembly.Location).FullName,
                       FFPLAY_DEFAULT_SDP_PATH);
                   using var media = new Media(_libVlc, new Uri(sdpFullPath));
                   _mediaPlayer.Play(media);
                   
                   //if (_activePeerConnection != null)
                   {
                       // request key frames
                       var localVideoSsrc = _activePeerConnection.VideoLocalTrack.Ssrc;
                       var remoteVideoSsrc = _activePeerConnection.VideoRemoteTrack.Ssrc;
                       RTCPFeedback pli = new RTCPFeedback(localVideoSsrc, remoteVideoSsrc, PSFBFeedbackTypesEnum.PLI);
                       _activePeerConnection.SendRtcpFeedback(SDPMediaTypesEnum.video, pli);

                   }
                   pc.OnRtpPacketReceived += (rep, media, rtpPkt) =>
                   {
                       if (media == SDPMediaTypesEnum.audio && rtpSession.AudioDestinationEndPoint != null)
                       {
                           //logger.LogDebug($"Forwarding {media} RTP packet to ffplay timestamp {rtpPkt.Header.Timestamp}.");
                           rtpSession.SendRtpRaw(media, rtpPkt.Payload, rtpPkt.Header.Timestamp,
                               rtpPkt.Header.MarkerBit, rtpPkt.Header.PayloadType);
                       }
                       else if (media == SDPMediaTypesEnum.video && rtpSession.VideoDestinationEndPoint != null)
                       {
                           logger.LogInformation($"Forwarding {media} RTP packet to ffplay timestamp {rtpPkt.Header.Timestamp}.");
                           rtpSession.SendRtpRaw(media, rtpPkt.Payload, rtpPkt.Header.Timestamp,
                               rtpPkt.Header.MarkerBit, rtpPkt.Header.PayloadType);                          
                       }

                   };
                   pc.OnRtpClosed += (reason) => { 
                       
                       logger.LogInformation($"{reason}");
                       rtpSession.Close(reason);
                   };
                    pc.OnVideoFrameReceived += (rep,idx, bytes, format) =>
                    {
                      
                        logger.LogInformation(format.ToString());
                    };
                }
           };

           _activePeerConnection = pc;
           return pc;
        }



        private static RTPSession CreateRtpSession(List<SDPAudioVideoMediaFormat> audioFormats,
            List<SDPAudioVideoMediaFormat> videoFormats)
        {
            var rtpSession = new RTPSession(false, false, false, IPAddress.Loopback);
            bool hasAudio = false;
            bool hasVideo = false;
            if (audioFormats != null && audioFormats.Count > 0)
            {
                MediaStreamTrack audioTrack = new MediaStreamTrack(SDPMediaTypesEnum.audio, false, audioFormats,
                    MediaStreamStatusEnum.RecvOnly);
                rtpSession.addTrack(audioTrack);
                hasAudio = true;
            }

            if (videoFormats != null && videoFormats.Count > 0)
            {
                MediaStreamTrack videoTrack = new MediaStreamTrack(SDPMediaTypesEnum.video, false, videoFormats,
                    MediaStreamStatusEnum.RecvOnly);
                rtpSession.addTrack(videoTrack);
                hasVideo = true;
            }
            
            var sdpOffer = rtpSession.CreateOffer(null);
            
            if (hasAudio)
            {
                sdpOffer.Media.Single(x => x.Media == SDPMediaTypesEnum.audio).Port = FFPLAY_DEFAULT_AUDIO_PORT;
            }

            if (hasVideo)
            {
                sdpOffer.Media.Single(x => x.Media == SDPMediaTypesEnum.video).Port = FFPLAY_DEFAULT_VIDEO_PORT;
            }
            Console.WriteLine(sdpOffer);
            using (StreamWriter sw = new StreamWriter(FFPLAY_DEFAULT_SDP_PATH))
            {
                sw.Write(sdpOffer);
            }

            rtpSession.Start();
            if (hasAudio)
            {
                rtpSession.SetDestination(SDPMediaTypesEnum.audio, new IPEndPoint(IPAddress.Loopback, FFPLAY_DEFAULT_AUDIO_PORT),
                    new IPEndPoint(IPAddress.Loopback, FFPLAY_DEFAULT_AUDIO_PORT + 1));
            }
            if (hasVideo)
            {
                rtpSession.SetDestination(SDPMediaTypesEnum.video, new IPEndPoint(IPAddress.Loopback, FFPLAY_DEFAULT_VIDEO_PORT),
                    new IPEndPoint(IPAddress.Loopback, FFPLAY_DEFAULT_VIDEO_PORT + 1));
                
            }
            return rtpSession;
        }

        private static async Task WebSocketMessageReceived(RTCPeerConnection pc,
            string message)
        {
            try
            {
                if (pc.localDescription == null)
                {
                    logger.LogDebug("Offer SDP received.");
                    SDP remoteSdp = SDP.ParseSDPDescription(message);
                    foreach (var ann  in remoteSdp.Media)
                    {
                        MediaStreamTrack track = new MediaStreamTrack(ann.Media, false, ann.MediaFormats.Values.ToList(), MediaStreamStatusEnum.RecvOnly);
                        pc.addTrack(track);
                    }

                    pc.setRemoteDescription(new RTCSessionDescriptionInit{sdp = message,type = RTCSdpType.offer});
                    var answer = pc.createAnswer(null);
                    await pc.setLocalDescription(answer);
                    Console.WriteLine(answer.sdp);
                    //context.WebSocket.Send(answer.sdp);
                    
                }else if (pc.remoteDescription == null)
                {
                    logger.LogDebug("Answer SDP: " + message);
                    var result = pc.setRemoteDescription(new RTCSessionDescriptionInit { sdp = message, type = RTCSdpType.answer });
                    if (result != SetDescriptionResultEnum.OK)
                    {
                        logger.LogWarning($"Failed to set remote description {result}.");
                    }

                }
                else
                {
                    logger.LogDebug("ICE Candidate: " + message);

                    if (string.IsNullOrWhiteSpace(message) || message.Trim().ToLower() == SDP.END_ICE_CANDIDATES_ATTRIBUTE)
                    {
                        logger.LogDebug("End of candidates message received.");
                    }
                    else
                    {
                        var candInit = Newtonsoft.Json.JsonConvert.DeserializeObject<RTCIceCandidateInit>(message);
                        pc.addIceCandidate(candInit);
                    }
 
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }
        
        private static Microsoft.Extensions.Logging.ILogger AddConsoleLogger()
        {
            var serilogLogger = new LoggerConfiguration()
                .Enrich.FromLogContext()
                .MinimumLevel.Is(Serilog.Events.LogEventLevel.Debug)
                .WriteTo.Console()
                .CreateLogger();

            var factory = new SerilogLoggerFactory(serilogLogger);
            SIPSorcery.LogFactory.Set(factory);
            return factory.CreateLogger<Program>();
        }

    }
}