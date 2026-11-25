**Ocarina Reborn v1.1.7 — Stability & Smoothness Update**

**🛠️ Crash fixes**
- Fixed a crash while models load when a model's memory ran full (for example on the Gerudo Valley bridge as adult Link).
- Fixed a rare loading crash caused by two game threads running at the same time that must never overlap.

**⚡ Smoother play**
- Fewer stutters: the game no longer waits for GPU work it does not need when it reads its visibility tables. The biggest difference is when the GPU is the bottleneck, e.g. at high resolutions.
- Fewer area reloads at the edge of Castle Town Market: turning back and forth no longer makes the game reload the neighbouring area again and again.

**🎬 Movies**
- Movie sound no longer drifts behind the picture at high frame rates.
- Movies keep their 16:9 shape on screens narrower than 16:9.

**🎮 Gameplay & controls**
- Fixed a save that could stay stuck at 20:00 (first adult night) outside Hyrule Field. The clock runs again, and the Sun's Song works.
- Plus is now Tab only, no longer also Enter. Some controller software sends Enter when you press A, which opened the pause menu. Your old controls are updated automatically; a custom Plus key you chose yourself stays.

**ℹ️ Updating**
- Install over your existing version, then let the launcher update the game. This update rebuilds the game code once, so it takes a few minutes longer than usual.
- Saves, settings and shader caches stay.

**📊 Help us improve**
- If you notice stutters or crashes, press **Ctrl+F8** when it happens, close the game and create a **Debug Package** in the launcher.
- Post it in **#post-your-logs-here** with the area and what happened.

Thanks for testing! 💙
