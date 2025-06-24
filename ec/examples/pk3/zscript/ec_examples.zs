// message sending example: announce monster deaths
class AnnouncerEventHandler : EventHandler
{
	override void WorldThingDied (WorldEvent e)
	{
		EventHandler.SendNetworkCommand("external/out/WorldThingDied", NET_STRING, e.Thing.GetClassName());
	}
}

// message sending example called by the in-game light switch through ACS
class LightSwitch
{
	static void Set (Actor activator, bool on)
	{
		EventHandler.SendNetworkCommand("external/out/LightSwitch/Set", NET_INT, on);
	}
}

// message receiving example: render text to a texture
class LiveScreenEventHandler : EventHandler
{
	const frameX = 6;
	const frameY = 6;
	const frameWidth = 52;
	const frameHeight = 23;

	transient TextureID screenFrameTexId;
	transient TextureID screenShineTexId;

	override void OnRegister()
	{
		screenFrameTexId = TexMan.CheckForTexture("MSCRN1F"); // frame
		screenShineTexId = TexMan.CheckForTexture("MSCRN1S"); // shine

		renderText("MSCRNA", "Hello World");
	}

	private void renderText(String textureName, String text)
	{
		TextureID texId = TexMan.CheckForTexture(textureName);
		let [w, h] = TexMan.GetSize(texId);
		Canvas cv = TexMan.GetCanvas(textureName);

		int scale = w / 64;
		int frameX = self.frameX * scale;
		int frameY = self.frameY * scale;
		int frameWidth = self.frameWidth * scale;
		int frameHeight = self.frameHeight * scale;

		// Set up frame bevel and empty canvas
		cv.DrawTexture(screenFrameTexId, false, 0, 0, DTA_ScaleX, scale, DTA_ScaleY, scale);
		cv.SetClipRect(frameX, frameY, frameWidth, frameHeight);
		cv.Clear(frameX, frameY, frameX + frameWidth, frameY + frameHeight, 0x000000);

		// Draw some text
		cv.DrawText(newSmallFont, Font.CR_GOLD, frameX, frameY, text);

		// Final touches, dim the screen a little and add a glossy shine
		cv.Dim(0xa0a0a0, 0.12, frameX, frameY, frameWidth, frameHeight);
		cv.DrawTexture(screenShineTexId, false, 0, 0, DTA_ScaleX, scale, DTA_ScaleY, scale);
	}

	override void NetworkCommandProcess(NetworkCommand cmd)
	{
		if (cmd.Command == "external/in/screenMessage")
		{
			string msg = cmd.ReadString();
			renderText("MSCRNA", msg);
		}
	}
}
