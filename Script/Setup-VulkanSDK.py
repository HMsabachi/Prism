import os
import sys
import time
from pathlib import Path

import requests
from fake_useragent import UserAgent
import colorama
from colorama import Back, Style

# --- Section: Config ---

VULKAN_SDK = os.environ.get('VULKAN_SDK')
PRISM_REQUIRED_VULKAN_VERSIONS = ('1.3.', '1.4.') # any 1.3.x or 1.4.x release is fine (Hazel Vulkan port gates on VK_API_VERSION_1_2, of which 1.3/1.4 are supersets)
PRISM_INSTALL_VULKAN_VERSION = '1.4.321.1'        # install this exact one if no supported version is present
VULKAN_SDK_INSTALLER_URL = f'https://sdk.lunarg.com/sdk/download/{PRISM_INSTALL_VULKAN_VERSION}/windows/VulkanSDK-{PRISM_INSTALL_VULKAN_VERSION}-Installer.exe'

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
VULKAN_SDK_LOCAL_PATH = REPO_ROOT / 'Prism' / 'vendor' / 'VulkanSDK'      # download cache, gitignored
VULKAN_SDK_INSTALLER_PATH = VULKAN_SDK_LOCAL_PATH / f'VulkanSDK-{PRISM_INSTALL_VULKAN_VERSION}-Installer.exe'

colorama.init()


# --- Section: Helpers (inlined from Hazel Utils.py) ---

def _get_user_agent():
    try:
        return UserAgent().random
    except Exception:
        return 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 PrismEngine/1.0'


def DownloadFile(url, filepath):
    with open(filepath, 'wb') as f:
        print('Waiting for response...')
        headers = {'User-Agent': _get_user_agent()}
        response = requests.get(url, stream=True, headers=headers, timeout=30)
        response.raise_for_status()
        total = response.headers.get('content-length')
        print('Downloading...')
        if total is None:
            f.write(response.content)
        else:
            downloaded = 0
            total = int(total)
            startTime = time.time()
            for data in response.iter_content(chunk_size=max(int(total / 1000), 1024 * 1024)):
                downloaded += len(data)
                f.write(data)
                done = int(50 * downloaded / total)
                percentage = (downloaded / total) * 100
                elapsedTime = time.time() - startTime
                avgKBPerSecond = (downloaded / 1024) / elapsedTime if elapsedTime > 0 else 0
                avgSpeedString = '{:.2f} KB/s'.format(avgKBPerSecond)
                if avgKBPerSecond > 1024:
                    avgMBPerSecond = avgKBPerSecond / 1024
                    avgSpeedString = '{:.2f} MB/s'.format(avgMBPerSecond)
                sys.stdout.write('\r[{}{}] {:.2f}% ({})     '.format('█' * done, '.' * (50 - done), percentage, avgSpeedString))
                sys.stdout.flush()
    sys.stdout.write('\n')


def YesOrNo():
    while True:
        reply = str(input('[Y/N]: ')).lower().strip()
        if reply[:1] == 'y':
            return True
        if reply[:1] == 'n':
            return False


# --- Section: Install ---

def InstallVulkanSDK():
    if sys.platform != 'win32':
        # TODO: automated Linux tarball download + extract once Prism targets Linux build
        print(f"{Style.BRIGHT}{Back.YELLOW}Automatic Vulkan SDK install is only supported on Windows.{Style.RESET_ALL}")
        print('For Linux/macOS, download and install manually:')
        print('  https://vulkan.lunarg.com/sdk/home')
        print('After install, set the VULKAN_SDK env var to the SDK root and re-run this script.')
        return

    VULKAN_SDK_LOCAL_PATH.mkdir(parents=True, exist_ok=True)
    print('Downloading {} to {}'.format(VULKAN_SDK_INSTALLER_URL, VULKAN_SDK_INSTALLER_PATH))
    DownloadFile(VULKAN_SDK_INSTALLER_URL, str(VULKAN_SDK_INSTALLER_PATH))
    print('Running Vulkan SDK installer...')
    print(f"{Style.BRIGHT}{Back.YELLOW}Make sure to install shader debug libs if you want to build in Debug!")
    os.startfile(str(VULKAN_SDK_INSTALLER_PATH))
    print(f"{Style.BRIGHT}{Back.RED}Re-run this script after installation{Style.RESET_ALL}")


def InstallVulkanPrompt():
    print('Would you like to install the Vulkan SDK?')
    if YesOrNo():
        InstallVulkanSDK()
        if sys.platform == 'win32':
            sys.exit(0)


# --- Section: Check ---

def CheckVulkanSDK():
    if VULKAN_SDK is None:
        print(f"{Style.BRIGHT}{Back.RED}You don't have the Vulkan SDK installed! (VULKAN_SDK env var not set){Style.RESET_ALL}")
        InstallVulkanPrompt()
        return False
    elif not any(v in VULKAN_SDK for v in PRISM_REQUIRED_VULKAN_VERSIONS):
        print(f'Located Vulkan SDK at {VULKAN_SDK}')
        print(f"{Style.BRIGHT}{Back.RED}Incorrect Vulkan SDK version! (Prism requires 1.3.x or 1.4.x){Style.RESET_ALL}")
        InstallVulkanPrompt()
        return False

    print(f"{Style.BRIGHT}{Back.GREEN}Correct Vulkan SDK located at {VULKAN_SDK}{Style.RESET_ALL}")
    return True


def CheckVulkanSDKDebugLibs():
    if VULKAN_SDK is None:
        return False
    if sys.platform != 'win32':
        # TODO: Linux debug-libs check (libshaderc_sharedd.a) once Prism targets Linux build
        print(f"{Style.BRIGHT}{Back.YELLOW}Debug libs check skipped on non-Windows (not yet supported).{Style.RESET_ALL}")
        return True

    shadercLib = Path(f'{VULKAN_SDK}/Lib/shaderc_sharedd.lib')
    if not shadercLib.exists():
        print(f"{Style.BRIGHT}{Back.YELLOW}Warning: No Vulkan SDK debug libs found. (Checked {shadercLib})")
        print(f"{Back.RED}Debug builds are not possible.{Style.RESET_ALL}")
        return False

    return True


# --- Section: Entry ---

def main():
    print(f"{Style.BRIGHT}Prism - Vulkan SDK Environment Check{Style.RESET_ALL}")
    print(f'Required: Vulkan SDK 1.3.x or 1.4.x (install version {PRISM_INSTALL_VULKAN_VERSION})')
    print()

    if not CheckVulkanSDK():
        return 1

    if CheckVulkanSDKDebugLibs():
        print(f"{Style.BRIGHT}{Back.GREEN}Vulkan SDK debug libs located.{Style.RESET_ALL}")

    return 0


if __name__ == '__main__':
    sys.exit(main())
