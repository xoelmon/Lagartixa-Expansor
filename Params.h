#pragma once

// IDs de todos los parámetros (procesador, interfaz y automatización del DAW)
namespace ids
{
    // Bandas de ancho estéreo
    inline constexpr const char* xoverLo = "xover_lo";
    inline constexpr const char* xoverHi = "xover_hi";
    inline constexpr const char* wLo     = "w_lo";
    inline constexpr const char* wMid    = "w_mid";
    inline constexpr const char* wHi     = "w_hi";

    // Espacio (difusión) y Haas
    inline constexpr const char* spaceAmt = "space_amt";
    inline constexpr const char* spaceHp  = "space_hp";
    inline constexpr const char* haasMs   = "haas_ms";
    inline constexpr const char* haasAmt  = "haas_amt";

    // Fase
    inline constexpr const char* phaseDeg = "phase_deg";
    inline constexpr const char* invL     = "inv_l";
    inline constexpr const char* invR     = "inv_r";
    inline constexpr const char* swapLR   = "swap_lr";

    // Salida
    inline constexpr const char* mix      = "mix";
    inline constexpr const char* outGain  = "out_gain";
    inline constexpr const char* monoCheck = "mono_check";
}
