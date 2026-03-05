#!/usr/bin/env bash
#
# Script de instalacion limpia para Eclipse Paho MQTT C y C++.
# Objetivo: terminar con las versiones reales publicadas en las refs seleccionadas.
#
# Requisitos funcionales cubiertos:
# 1) Verifica que se ejecute en Linux y con Bash.
# 2) Detecta una instalacion previa, la elimina y reinstala.
# 3) Ejecuta validaciones de pre/post instalacion para confirmar version final.

set -Eeuo pipefail

# Versiones objetivo en github
readonly PAHO_C_VERSION="v1.3.14"
readonly PAHO_CPP_VERSION_REQUESTED="v1.3.14"
readonly PAHO_CPP_VERSION_FALLBACK="v1.3.2"
readonly PAHO_C_REPO="https://github.com/UnInformaticoAburrido/paho.mqtt.c.git"
readonly PAHO_C_FALLBACK_REPO="https://github.com/eclipse/paho.mqtt.c.git"
readonly PAHO_CPP_REPO="https://github.com/eclipse/paho.mqtt.cpp.git"
TARGET_C_VERSION_NO_V=""
TARGET_CPP_VERSION_NO_V=""
WORK_DIR=""

log_info() { printf '[INFO] %s\n' "$*"; }
log_warn() { printf '[WARN] %s\n' "$*" >&2; }
log_error() { printf '[ERROR] %s\n' "$*" >&2; }

die() {
    log_error "$*"
    exit 1
}

cleanup() {
    # Evita error con set -u si el script falla antes de inicializar el directorio temporal.
    if [[ -n "${WORK_DIR:-}" && -d "${WORK_DIR:-}" ]]; then
        rm -rf "${WORK_DIR:?}"
    fi
}

trap cleanup EXIT

run_root() {
    # Ejecuta como root si ya se tiene ese usuario; si no, usa sudo.
    if [[ "${EUID}" -eq 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

assert_linux_bash() {
    # Requisito 1: entorno Linux + Bash.
    [[ -n "${BASH_VERSION:-}" ]] || die "Este script requiere Bash."
    [[ "$(uname -s)" == "Linux" ]] || die "Este script solo funciona en Linux."
    command -v apt-get >/dev/null 2>&1 || die "No se encontro apt-get (se espera Debian/Ubuntu)."
}

assert_required_tools() {
    local missing=()
    local cmds=(git cmake pkg-config g++ ldconfig)
    local cmd
    for cmd in "${cmds[@]}"; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            missing+=("$cmd")
        fi
    done

    if ((${#missing[@]} > 0)); then
        log_warn "Faltan herramientas: ${missing[*]}"
        log_info "Intentando instalar dependencias..."
        run_root apt-get update
        run_root apt-get install -y git build-essential cmake libssl-dev pkg-config
    fi
}

has_previous_installation() {
    # Detecta instalaciones previas por paquetes y por artefactos locales.
    if dpkg -l 2>/dev/null | grep -qiE 'paho|mqttpp'; then
        return 0
    fi
    if compgen -G "/usr/local/lib/libpaho-mqtt*.so*" >/dev/null; then
        return 0
    fi
    if compgen -G "/usr/local/lib64/libpaho-mqtt*.so*" >/dev/null; then
        return 0
    fi
    if [[ -d "/usr/local/include/mqtt" ]] || compgen -G "/usr/local/include/MQTT*.h" >/dev/null; then
        return 0
    fi
    return 1
}

remove_previous_installation() {
    #si existe instalacion previa, se elimina completamente.
    log_info "Eliminando instalaciones previas de Paho MQTT..."

    # Purga de paquetes instalados por apt (si existen).
    run_root apt-get remove --purge -y 'libpaho-mqtt*' || true
    run_root apt-get autoremove -y || true

    # Limpieza de posibles instalaciones manuales en /usr/local.
    run_root rm -f /usr/local/lib/libpaho-mqtt*.so* || true
    run_root rm -f /usr/local/lib64/libpaho-mqtt*.so* || true
    run_root rm -f /usr/local/lib/pkgconfig/paho-mqtt*.pc || true
    run_root rm -f /usr/local/lib64/pkgconfig/paho-mqtt*.pc || true
    run_root rm -rf /usr/local/include/mqtt || true
    run_root rm -f /usr/local/include/MQTT*.h || true

    run_root ldconfig
}

remote_has_tag() {
    local repo_url="$1"
    local ref="$2"
    git ls-remote --exit-code --tags "$repo_url" "refs/tags/$ref" >/dev/null 2>&1
}

remote_has_branch() {
    local repo_url="$1"
    local ref="$2"
    git ls-remote --exit-code --heads "$repo_url" "refs/heads/$ref" >/dev/null 2>&1
}

clone_checkout_ref() {
    # Clona y valida una referencia exacta por tipo (tag o branch).
    local repo_url="$1"
    local ref="$2"
    local ref_type="$3"
    local dest="$4"

    git -c advice.detachedHead=false clone --depth 1 --branch "$ref" "$repo_url" "$dest" >/dev/null

    if [[ "$ref_type" == "tag" ]]; then
        local checked_tag
        checked_tag="$(git -C "$dest" describe --tags --exact-match 2>/dev/null || true)"
        [[ "$checked_tag" == "$ref" ]] || die "Tag incorrecto en $dest: esperado $ref, encontrado $checked_tag"
    else
        local checked_branch
        checked_branch="$(git -C "$dest" branch --show-current)"
        [[ "$checked_branch" == "$ref" ]] || die "Rama incorrecta en $dest: esperada $ref, encontrada $checked_branch"
    fi
}

clone_ref_prefer_tag_then_branch() {
    # Devuelve por stdout el tipo de referencia usada: tag|branch.
    local repo_url="$1"
    local ref="$2"
    local dest="$3"

    if remote_has_tag "$repo_url" "$ref"; then
        clone_checkout_ref "$repo_url" "$ref" "tag" "$dest"
        echo "tag"
        return 0
    fi
    if remote_has_branch "$repo_url" "$ref"; then
        clone_checkout_ref "$repo_url" "$ref" "branch" "$dest"
        echo "branch"
        return 0
    fi
    return 1
}

clone_ref_with_repo_fallback() {
    # Intenta clonar una ref por tag/rama en repo primario y, si no existe, en fallback.
    local primary_repo="$1"
    local fallback_repo="$2"
    local ref="$3"
    local dest="$4"
    local component="$5"

    local used_type
    if used_type="$(clone_ref_prefer_tag_then_branch "$primary_repo" "$ref" "$dest")"; then
        log_info "$component: usando $used_type '$ref' desde repo primario."
        return 0
    fi

    log_warn "$component: el repo primario no contiene tag/rama '$ref'. Se usa fallback."
    if used_type="$(clone_ref_prefer_tag_then_branch "$fallback_repo" "$ref" "$dest")"; then
        log_info "$component: usando $used_type '$ref' desde fallback."
        return 0
    fi

    die "$component: no existe tag/rama '$ref' ni en repo primario ni en fallback."
}

install_paho_c() {
    local src_dir="$1/paho.mqtt.c"
    log_info "Instalando Paho MQTT C $PAHO_C_VERSION..."

    clone_ref_with_repo_fallback "$PAHO_C_REPO" "$PAHO_C_FALLBACK_REPO" "$PAHO_C_VERSION" "$src_dir" "Paho MQTT C"

    TARGET_C_VERSION_NO_V="$(detect_c_source_version "$src_dir" || true)"
    [[ -n "$TARGET_C_VERSION_NO_V" ]] || die "No fue posible detectar version fuente de Paho MQTT C."
    log_info "Paho MQTT C: version fuente detectada ${TARGET_C_VERSION_NO_V}."

    cmake -Wno-dev -Wno-deprecated -S "$src_dir" -B "$src_dir/build" \
        -DPAHO_WITH_SSL=ON \
        -DPAHO_BUILD_SHARED=ON \
        -DPAHO_ENABLE_TESTING=OFF

    cmake --build "$src_dir/build" --parallel "$(nproc)"
    run_root cmake --install "$src_dir/build"
    run_root ldconfig
}

install_paho_cpp() {
    local src_dir="$1/paho.mqtt.cpp"
    local selected_cpp_ref="$PAHO_CPP_VERSION_REQUESTED"
    log_info "Instalando Paho MQTT C++ (solicitado: $PAHO_CPP_VERSION_REQUESTED)..."

    if clone_ref_prefer_tag_then_branch "$PAHO_CPP_REPO" "$selected_cpp_ref" "$src_dir" >/dev/null; then
        TARGET_CPP_VERSION_NO_V="${selected_cpp_ref#v}"
    else
        selected_cpp_ref="$PAHO_CPP_VERSION_FALLBACK"
        log_warn "Paho MQTT C++: no existe '$PAHO_CPP_VERSION_REQUESTED' en upstream. Se usara '$selected_cpp_ref'."
        clone_ref_prefer_tag_then_branch "$PAHO_CPP_REPO" "$selected_cpp_ref" "$src_dir" >/dev/null \
            || die "Paho MQTT C++: tampoco existe '$selected_cpp_ref' en upstream."
        TARGET_CPP_VERSION_NO_V="${selected_cpp_ref#v}"
    fi

    cmake -Wno-dev -Wno-deprecated -S "$src_dir" -B "$src_dir/build" \
        -DPAHO_BUILD_SHARED=ON \
        -DPAHO_WITH_MQTT_C=OFF \
        -DPAHO_WITH_SSL=ON \
        -DPAHO_BUILD_STATIC=OFF \
        -DPAHO_BUILD_SAMPLES=OFF \
        -DPAHO_BUILD_TESTS=OFF

    cmake --build "$src_dir/build" --parallel "$(nproc)"
    run_root cmake --install "$src_dir/build"
    run_root ldconfig
}

detect_version_from_cmake_config() {
    # Obtiene PACKAGE_VERSION desde un archivo *ConfigVersion.cmake instalado.
    local file="$1"
    [[ -f "$file" ]] || return 1
    awk -F'"' '/set\(PACKAGE_VERSION[[:space:]]+"/{print $2; exit}' "$file"
}

detect_version_from_so_name() {
    # Extrae x.y.z desde nombres tipo lib*.so.x.y.z
    local pattern="$1"
    local path
    for path in $pattern; do
        if [[ "$path" =~ \.so\.([0-9]+\.[0-9]+\.[0-9]+)$ ]]; then
            echo "${BASH_REMATCH[1]}"
            return 0
        fi
    done
    return 1
}

detect_c_source_version() {
    # Detecta la version real declarada por el fuente de paho.mqtt.c.
    local src_dir="$1"
    local major minor patch

    if [[ -f "$src_dir/version.major" && -f "$src_dir/version.minor" && -f "$src_dir/version.patch" ]]; then
        major="$(tr -dc '0-9' < "$src_dir/version.major")"
        minor="$(tr -dc '0-9' < "$src_dir/version.minor")"
        patch="$(tr -dc '0-9' < "$src_dir/version.patch")"
        if [[ -n "$major" && -n "$minor" && -n "$patch" ]]; then
            echo "${major}.${minor}.${patch}"
            return 0
        fi
    fi

    awk '
        match($0, /VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+)/, m) {
            print m[1]
            exit
        }
    ' "$src_dir/CMakeLists.txt"
}

detect_c_installed_version() {
    local v=""
    v="$(detect_version_from_cmake_config /usr/local/lib/cmake/eclipse-paho-mqtt-c/eclipse-paho-mqtt-cConfigVersion.cmake || true)"
    [[ -n "$v" ]] || v="$(detect_version_from_cmake_config /usr/local/lib64/cmake/eclipse-paho-mqtt-c/eclipse-paho-mqtt-cConfigVersion.cmake || true)"
    [[ -n "$v" ]] || v="$(detect_version_from_so_name '/usr/local/lib/libpaho-mqtt3as.so.* /usr/local/lib/libpaho-mqtt3a.so.* /usr/local/lib64/libpaho-mqtt3as.so.* /usr/local/lib64/libpaho-mqtt3a.so.*' || true)"
    [[ -n "$v" ]] || return 1
    echo "$v"
}

detect_cpp_installed_version() {
    local v=""
    # Algunas versiones no publican paho-mqttpp3.pc; por eso pkg-config es opcional.
    v="$(pkg-config --modversion paho-mqttpp3 2>/dev/null || true)"
    [[ -n "$v" ]] || v="$(detect_version_from_cmake_config /usr/local/lib/cmake/PahoMqttCpp/PahoMqttCppConfigVersion.cmake || true)"
    [[ -n "$v" ]] || v="$(detect_version_from_cmake_config /usr/local/lib64/cmake/PahoMqttCpp/PahoMqttCppConfigVersion.cmake || true)"
    [[ -n "$v" ]] || v="$(detect_version_from_so_name '/usr/local/lib/libpaho-mqttpp3.so.* /usr/local/lib64/libpaho-mqttpp3.so.*' || true)"
    [[ -n "$v" ]] || return 1
    echo "$v"
}

detect_c_link_lib() {
    # Devuelve el nombre (sin prefijo/sufijo) de una variante C disponible para enlace.
    local libs=(paho-mqtt3as paho-mqtt3cs paho-mqtt3a paho-mqtt3c)
    local lib
    for lib in "${libs[@]}"; do
        if ldconfig -p | grep -q "lib${lib}\\.so"; then
            echo "$lib"
            return 0
        fi
    done
    return 1
}

validate_shared_objects() {
    # Valida que el linker vea las bibliotecas.
    ldconfig -p | grep -q 'libpaho-mqttpp3\.so' || die "No se encontro libpaho-mqttpp3.so en el linker cache."
    ldconfig -p | grep -qE 'libpaho-mqtt3(a|c|as|cs)\.so' || die "No se encontro ninguna libreria Paho MQTT C en el linker cache."
}

validate_header_versions() {
    # Valida que los headers principales de C y C++ existan.
    # La version se valida por ConfigVersion/soname, no por headers,
    # porque en paho.mqtt.cpp v1.3.2 no existe mqtt/version.h.
    [[ -f /usr/local/include/MQTTAsync.h ]] \
        || die "No existe /usr/local/include/MQTTAsync.h"
    [[ -f /usr/local/include/mqtt/async_client.h ]] \
        || die "No existe /usr/local/include/mqtt/async_client.h"
}

run_compile_smoke_test() {
    # Prueba de compilacion+enlace para asegurar headers y libs operativos.
    local tmp_dir="$1"
    local c_lib="$2"
    local test_src="$tmp_dir/smoke_test.cpp"
    local test_bin="$tmp_dir/smoke_test"

    cat >"$test_src" <<'EOF'
    #include <MQTTAsync.h>
    #include <mqtt/async_client.h>

int main() {
    mqtt::async_client cli("tcp://localhost:1883", "codex-smoke-test-client");
    (void)cli;
    return 0;
}
EOF
    g++ -std=c++17 "$test_src" -o "$test_bin" \
        -I/usr/local/include \
        -L/usr/local/lib -L/usr/local/lib64 \
        -Wl,-rpath,/usr/local/lib -Wl,-rpath,/usr/local/lib64 \
        -lpaho-mqttpp3 "-l${c_lib}"
    "$test_bin"
}

validate_final_state() {
    # Requisito 3: tests pertinentes para verificar estado final.
    log_info "Ejecutando validaciones post-instalacion..."

    [[ -n "$TARGET_CPP_VERSION_NO_V" ]] || die "Version objetivo de Paho MQTT C++ no inicializada."

    local cpp_found
    cpp_found="$(detect_cpp_installed_version || true)"
    [[ -n "$cpp_found" ]] || die "No fue posible detectar version instalada de Paho MQTT C++."
    [[ "$cpp_found" == "$TARGET_CPP_VERSION_NO_V" ]] \
        || die "Version incorrecta en Paho MQTT C++: esperado $TARGET_CPP_VERSION_NO_V, encontrado $cpp_found"

    local c_found
    [[ -n "$TARGET_C_VERSION_NO_V" ]] || die "Version objetivo de Paho MQTT C no inicializada."
    c_found="$(detect_c_installed_version || true)"
    [[ -n "$c_found" ]] || die "No fue posible detectar version instalada de Paho MQTT C."
    [[ "$c_found" == "$TARGET_C_VERSION_NO_V" ]] \
        || die "Version incorrecta en Paho MQTT C: esperado $TARGET_C_VERSION_NO_V, encontrado $c_found"

    local c_lib_for_link
    c_lib_for_link="$(detect_c_link_lib || true)"
    [[ -n "$c_lib_for_link" ]] || die "No se encontro ninguna libreria C de Paho para prueba de enlace."

    validate_shared_objects
    validate_header_versions
    run_compile_smoke_test "$1" "$c_lib_for_link"

    log_info "OK: sistema validado con Paho MQTT C ${TARGET_C_VERSION_NO_V} y C++ ${TARGET_CPP_VERSION_NO_V}."
}

main() {
    assert_linux_bash
    assert_required_tools

    if has_previous_installation; then
        log_info "Se detecto una version previa. Se eliminara antes de instalar."
        remove_previous_installation
    else
        log_info "No se detecto version previa. Se realizara instalacion limpia."
    fi

    WORK_DIR="$(mktemp -d /tmp/paho-install-XXXXXX)"

    install_paho_c "$WORK_DIR"
    install_paho_cpp "$WORK_DIR"
    validate_final_state "$WORK_DIR"
}

main "$@"
