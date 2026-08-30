#!/usr/bin/env python3

# Import standard libraries.
import os
import time

# Import 3rd-party packages.
import pyautogui as pag


def sleep(sec_sleep: float) -> None:
    """
    Sleep specified seconds.
    """
    time.sleep(sec_sleep)


def typekeys(text: str, sec_between_keys: float = 0.1, sec_sleep: float = 0.5) -> None:
    """
    Type keys.
    """
    pag.typewrite(text, interval=sec_between_keys)
    time.sleep(sec_sleep)


def hotkeys(*keys, sec_sleep: float = 0.5) -> None:
    """
    Type a key with additional modifiers.
    """
    pag.hotkey(*keys)
    time.sleep(sec_sleep)


def main() -> None:
    """
    Entrypoint of this script.
    """
    input("Hit some key when you started recording...")

    for sec in [3, 2, 1]:
        print(f"Demonstration will starts within {sec} second...")
        sleep(1.0)

    typekeys("rm -f ~/.bash_history\n")
    typekeys("clear\n")
    sleep(1.0)

    typekeys("ls sou")
    hotkeys("ctrl", "i")
    typekeys("cx")
    hotkeys("ctrl", "i")
    typekeys("main.c")
    hotkeys("ctrl", "i")
    typekeys("\n")
    sleep(1.0)

    typekeys("ls ")
    hotkeys("ctrl", "f")
    typekeys("jjj")
    typekeys("kkk")
    typekeys("jjjljl")
    typekeys("/")
    typekeys("main\n")
    typekeys("\n")
    typekeys("\n")
    sleep(1.0)

    typekeys("ps -p ")
    hotkeys("ctrl", "p")
    typekeys("/")
    typekeys("exbash")
    typekeys("\n")
    typekeys("\n")
    typekeys("\n")
    sleep(1.0)

    typekeys("git ")
    typekeys("st")
    hotkeys("ctrl", "i")
    typekeys("\n")
    sleep(1.0)

    typekeys("echo 'welcome to ExBash!'")
    typekeys("\n")
    sleep(1.0)

    typekeys("clear\n")
    sleep(1.0)


if __name__ == "__main__":
    main()


# vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
