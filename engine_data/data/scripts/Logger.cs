using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public static class Log
{
    public static void Trace(string message,
                                [CallerMemberName] string func = "",
                                [CallerFilePath] string file = "",
                                [CallerLineNumber] int line = 0)
    {
        Internal_LogTrace(message, func, file, line);
    }
    public static void Info(string message,
                               [CallerMemberName] string func = "",
                               [CallerFilePath] string file = "",
                               [CallerLineNumber] int line = 0)
    {
        Internal_LogInfo(message, func, file, line);
    }
    public static void Warning(string message,
                                  [CallerMemberName] string func = "",
                                  [CallerFilePath] string file = "",
                                  [CallerLineNumber] int line = 0)
    {
        Internal_LogWarning(message, func, file, line);
    }
    public static void Error(string message,
                                [CallerMemberName] string func = "",
                                [CallerFilePath] string file = "",
                                [CallerLineNumber] int line = 0)
    {
        Internal_LogError(message, func, file, line);
    }


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogTrace(string message, string func, string file, int line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogInfo(string message, string func, string file, int line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogWarning(string message, string func, string file, int line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogError(string message, string func, string file, int line);

}



}

}


