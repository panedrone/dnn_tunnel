using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

public enum Camera_Backend_API
{
    ANY = 0xff,
    DSHOW = 0,
    MSMF = 1
}

public class AOI
{
    public static readonly AOI Empty = new AOI();

    public int X;
    public int Y;
    public int Width;
    public int Height;

    public AOI()
    {
        X = 0;
        Y = 0;
        Width = 0;
        Height = 0;
    }

    public AOI(int x, int y, int width, int height)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
    }
}

public class Tunnel_Items_Finder
{
    ////////////////////////////////////////////////////////////////////
    ///
    // stat queried on timer
    //

    public static void get_stat(out long max_track_age, out double avg_track_length, out double avg_track_age, out double fps)
    {
        max_track_age = _get_max_track_age();
        avg_track_length = _get_avg_track_length();
        avg_track_age = _get_avg_track_age();
        fps = _get_fps();
    }

    ////////////////////////////////////////////////////////////////////
    //
    // mandatory settings before start
    //

    public static void init_tracking(string yolo_class_names, string yolo_cfg, string yolo_weights,
        int inp_width_height, AOI aoi, bool flip_x, bool rgb, bool half_size_frame)
    {
        _set_on_message_callback((msg) =>
        {
            post_message(msg);
        });

        _set_on_item_callback((bytes_count, ptr) =>
        {
            if (on_item != null)
            {
                byte[] img_bytes = new byte[bytes_count];
                Marshal.Copy(ptr, img_bytes, 0, bytes_count);
                on_item(img_bytes);
            }
        });

        /////////////////////////////////////

        _setup_yolo(yolo_class_names, yolo_cfg, yolo_weights, rgb);

        _setup_inp_width_height(inp_width_height);

        _set_aoi(aoi.X, aoi.Y, aoi.Width, aoi.Height);

        _set_half_size_frame(half_size_frame);

        _set_flip_x(flip_x);
    }

    ////////////////////////////////////////////////////////////////////
    ///
    // settings
    //

    public static void set_dnn_thresholds(int conf, int nms)
    {
        _set_dnn_thresholds(conf, nms);
    }

    public static void set_aoi(AOI aoi)
    {
        _set_aoi(aoi.X, aoi.Y, aoi.Width, aoi.Height);
    }

    public static void set_flip_x(bool value)
    {
        _set_flip_x(value);
    }

    public static void set_mask(string path)
    {
        _set_mask(path);
    }

    public static void set_min_bounding_box(int value)
    {
        _set_min_bounding_box(value);
    }

    public static void set_rect_distance_ratio(double value)
    {
        _set_rect_distance_ratio(value);
    }

    public static void set_frame_delay(int value)
    {
        _set_frame_delay(value);
    }

    public static void set_half_size_frame(bool value)
    {
        _set_half_size_frame(value);
    }

    //////////////////////////////////////////////////////////////////

    public static Action<byte[]> on_item = null;

    public static Action<string> on_message = null;

    //////////////////////////////////////////////////////////////////

    public static void run_cam_tracking(int cam_index)
    {
        string exe_dir = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase.Substring(5));

        while (exe_dir.StartsWith("\\"))
        {
            exe_dir = exe_dir.Substring(1);
        }

        string[] lines = File.ReadAllLines(Path.Combine(exe_dir, "yolo-v3_tiny_160_1_rgb_blur_only.txt"));

        string yolo_class_names = Path.Combine(exe_dir, lines[0]);
        string yolo_cfg = Path.Combine(exe_dir, lines[1]);
        string yolo_weights = Path.Combine(exe_dir, lines[2]);

        int inp_width_height = 160;

        bool flip_x = false;

        AOI aoi = AOI.Empty;

        init_tracking(yolo_class_names, yolo_cfg, yolo_weights, inp_width_height, aoi, flip_x, rgb: true, half_size_frame: false);

        run_cam_tracking(cam_index, true, 0, 0, Camera_Backend_API.ANY, true, 50, 0);
    }

    public static void run_cam_tracking(int cam_index,
        bool cam_resolution_use_default, int cam_resolution_width, int cam_resolution_height,
        Camera_Backend_API api, bool debug, int conf_0_100, int nms_0_100)
    {
        if (_is_running())
        {
            post_message("[ERROR] Tracking is running");

            return;
        }

        Thread t = new Thread(() =>
        {
            try
            {
                _process_cam_flow(cam_index,
                    cam_resolution_use_default, cam_resolution_width, cam_resolution_height,
                    (int)api, debug, conf_0_100, nms_0_100, grayscale: false);

                post_message("Ended");
            }
            catch (Exception e)
            {
                post_message("[ERROR] Tracking failed: " + e.Message);
            }
        });

        t.Start();
    }

    public static void run_video_tracking(string vieo_file_path, bool debug, int conf_0_100, int nms_0_100)
    {
        if (_is_running())
        {
            post_message("[ERROR] Tracking is running");

            return;
        }

        Thread t = new Thread(() =>
        {
            try
            {
                _process_video_flow(vieo_file_path, debug, conf_0_100, nms_0_100, grayscale: false);

                post_message("Ended");
            }
            catch (Exception e)
            {
                post_message("[ERROR] Tracking failed: " + e.Message);
            }
        });

        t.Start();
    }

    public static void run_cam_free_run(int cam_index, bool cam_resolution_use_default, int cam_resolution_width, int cam_resolution_height,
         Camera_Backend_API api, bool grayscale)
    {
        if (_is_running())
        {
            post_message("[ERROR] Already running");

            return;
        }

        _set_on_message_callback((msg) =>
        {
            post_message(msg);
        });

        Thread t = new Thread(() =>
        {
            try
            {
                _process_free_run(cam_index, cam_resolution_use_default, cam_resolution_width, cam_resolution_height, (int)api, grayscale);

                post_message("Free Run Ended");
            }
            catch (Exception e)
            {
                post_message("[ERROR] Capturing Thread failed: " + e.Message);
            }
        });

        t.Start();
    }

    public static bool is_running()
    {
        return _is_running();
    }

    public static void stop()
    {
        _stop();
    }

    private static void post_message(string msg)
    {
        on_message?.Invoke(msg);
    }

    ////////////////////////////////////////////////////////////////////
    //
    // Mono https://www.mono-project.com/docs/advanced/pinvoke/
    //
    // If you have control over the library name, keep the above naming conventions in mind 
    // and don’t use a platform-specific library name in the DllImport statement. 
    // Instead, just use the library name itself, without any prefixes or suffixes, 
    // and rely on the runtime to find the appropriate library at runtime. For example:

    private const string DNN_TUNNEL_CV = "libdnn_tunnel_core"; // === panedrone: it works in VS too

    private const CallingConvention CALL_CONV = CallingConvention.Cdecl;

    ////////////////////////////////////////////////////////////////////

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern bool _is_running();

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern bool _stop();

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _process_cam_flow(int cam_index, bool cam_resolution_use_default, int cam_resolution_width, int cam_resolution_height,
        int api, bool debug, int conf_0_100, int nms_0_100, bool grayscale);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _process_video_flow(string vieo_file_path, bool debug, int conf_0_100, int nms_0_100, bool grayscale);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _process_free_run(int cam_index, bool cam_resolution_use_default, int cam_resolution_width, int cam_resolution_height,
        int api, bool grayscale);

    // stat functions called on each frame

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern long _get_max_track_age();

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern double _get_avg_track_length();

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern double _get_avg_track_age();

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern double _get_fps();

    // mandatory settings before start

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV, CharSet = CharSet.Ansi)]
    private static extern void _setup_yolo(string class_names, string yolo_cfg, string yolo_weights, bool rgb);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _setup_inp_width_height(int inp_width_height);

    // settings

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _set_dnn_thresholds(int conf_0_100, int nms_0_100);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _set_aoi(int x, int y, int w, int h);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private extern static void _set_flip_x(bool value);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV, CharSet = CharSet.Ansi)]
    private static extern void _set_mask(string path);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _set_min_bounding_box(int value);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    static extern void _set_rect_distance_ratio(double value);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _set_frame_delay(int value);

    [DllImport(DNN_TUNNEL_CV, CallingConvention = CALL_CONV)]
    private static extern void _set_half_size_frame(bool value);

    // callbacks

    // https://www.codeproject.com/Tips/318140/How-to-make-a-callback-to-Csharp-from-C-Cplusplus
    [UnmanagedFunctionPointer(CALL_CONV)]
    private delegate void on_item_callback(int bytes_count, IntPtr bytes);

    [UnmanagedFunctionPointer(CALL_CONV, CharSet = CharSet.Ansi)]
    private delegate void on_message_callback(string msg);

    [DllImport(DNN_TUNNEL_CV)]
    private static extern void _set_on_item_callback([MarshalAs(UnmanagedType.FunctionPtr)] on_item_callback callbackPointer);

    [DllImport(DNN_TUNNEL_CV)]
    private static extern void _set_on_message_callback([MarshalAs(UnmanagedType.FunctionPtr)] on_message_callback callbackPointer);
}