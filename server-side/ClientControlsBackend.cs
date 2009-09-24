using System;
using System.IO;
using System.Web;
using System.Text;
using System.Web.Security;
using System.Collections;
using System.Data;

namespace PhotoAlbum.Engine
{
	/// <summary>
	/// Summary description for ClientControlsBackend.
	/// </summary>
	public class ClientControlsBackend
	{
		private ClientControlsBackend()
		{
		}

		public static void HandleUploadControl(HttpRequest Request, HttpResponse Response)
		{
			Guid userId = Guid.Empty;
			try 
			{
				HttpCookie cookie = Request.Cookies.Get(FormsAuthentication.FormsCookieName);
				if(cookie != null && cookie.Value != null && cookie.Value.Length > 0) 
				{
					FormsAuthenticationTicket ticket = FormsAuthentication.Decrypt(cookie.Value);
					if(ticket != null && ticket.Name != null && ticket.Name.Length > 0)
						userId = new Guid(ticket.Name);
				}
				if(userId == Guid.Empty)
					userId = Principal.MemberIdOrZero;
			}
			catch(Exception exc)
			{
				Log.LogDebug(exc);
			}

			string v = Request["v"];
			int version = v!=null&&v.Length>0?int.Parse(v):0;
			string action = Request["action"];
			try
			{
				Response.ClearContent();

				if(action == "upload")
					HandleUploadControlUpload(Request, Response, userId);
				else if(action == "login")
					HandleUploadControlLogin(Request, Response);
				else if(action == "updatedetails")
					HandleDetailsUpdate(Request, Response, userId);
				else if(action == "userlookup")
					HandleUserLookup(Request, Response, userId);
				else if(action == "categorylookup")
					HandleCategoryLookup(Request, Response, userId);
				else if(action == "createcategory")
					HandleCreateCategory(Request, Response, userId);
				else
				{
					BinaryWriter w = new BinaryWriter(Response.OutputStream);
					w.Write(new byte[8]);
				}
			}
			finally
			{
				Response.Flush();
				Response.Close();
				Response.End();
			}
		}

		private static void HandleDetailsUpdate(HttpRequest Request, HttpResponse Response, Guid userId)
		{
			BinaryReader r			= new BinaryReader(Request.InputStream);
			int responseVersion		= 0;
			int responseCode		= -1;
			Encoding e				= Encoding.Unicode;
			try
			{
				int clientVersion	= r.ReadInt32();
				responseVersion		= clientVersion;
				if(userId == Guid.Empty) 
				{
					responseCode = -2;
					return;
				}
				int itemcount		= r.ReadInt32();
				using(Db db = new Db())
				{
					db.CommandText = "UPDATE tPhoto SET name=@title, comment=@text WHERE id=@photoId";
					IDataParameter pid		= db.AddParameter("@photoId",	Guid.Empty);
					IDataParameter ptitle	= db.AddParameter("@title",		string.Empty);
					IDataParameter ptext	= db.AddParameter("@text",		string.Empty);
					for(int i=0;i<itemcount;i++)
					{
						Guid photoId = new Guid(r.ReadBytes(16));
						int len;
						byte[] raw;
						len = r.ReadInt32();
						raw = r.ReadBytes(len);
						string title = e.GetString(raw, 0, raw.Length-2);
						
						len = r.ReadInt32();
						raw = r.ReadBytes(len);
						string text = e.GetString(raw, 0, raw.Length-2);

						bool ok;
						try
						{
							Database.EnforcePhotoPermission(userId, photoId, Permission.Change);
							pid.Value		= photoId;
							ptitle.Value	= title;
							ptext.Value		= text;
							ok = db.ExecuteNonQuery(0)==1;
						}
						catch(System.Data.SqlClient.SqlException exc)
						{
							string debug = exc.ToString();
							throw;
						}
						catch(Error_AccessDenied)
						{
							ok = false;
							Log.LogSecurity(2, "Denied access to edit photo details with client control. userId:{0}, photoId:{1}, title{2}, text:{3}.", 
								userId, photoId, title, text);
						}
					}
				}
				responseCode = 0;
				Log.LogStatistics(2, "Updated details for {0} photos.", itemcount);
			}
			catch
			{
				responseCode = -1;
				throw;
			}
			finally
			{
				BinaryWriter writer = new BinaryWriter(Response.OutputStream);

				writer.Write(responseVersion);
				writer.Write(responseCode);
				
				writer.Flush();
			}
		}

		private static string FixUsername(string username)
		{
			//Since the hashPassword function is case sensitive, recv the real thing from db.
			//HACK, this needs password recovery to be solved.
			using(Db db = new Db())
			{
				db.CommandText = "SELECT username FROM tMember WHERE lower(username) = @username";
				db.AddParameter("@username", username);
				username = db.ExecuteScalar() as string;
				return username;
			}
		}

		public enum LoginResultCode : int
		{
			LoginOk				= 0,
			LoginServerError	= -1,
			BadCredentials		= 1
		}

		private static void HandleUploadControlLogin(HttpRequest Request, HttpResponse Response)
		{
			Encoding e		= Encoding.Unicode;
			BinaryReader r	= new BinaryReader(Request.InputStream, e);

			int		messageVersion	= -1;
			LoginResultCode		resultCode	= LoginResultCode.LoginServerError;
			string	cookieName		= string.Empty;
			string	cookieValue		= string.Empty;
			string	message			= string.Empty;
			Guid	userId			= Guid.Empty;
			bool	loginOk			= false;

			const int PersistDays	= 30;
			const int timeoutMinutes= 30;

			try
			{
				int		len;
				byte[]	raw;

				// read version
				messageVersion	= r.ReadInt32();

				// read persistent
				bool persistent = r.ReadByte()==0?false:true;

				// read username
				len				= r.ReadInt32();
				raw				= r.ReadBytes(len);
				string username = e.GetString(raw, 0, raw.Length-2).Trim();

				// read password
				len				= r.ReadInt32();
				raw				= r.ReadBytes(len);
				string password = e.GetString(raw, 0, raw.Length-2).Trim();

				// do login
				username		= FixUsername(username);
				loginOk			= username==null?false:Database.ComparePassword(username, password, out userId);
				if(!loginOk)
				{
					resultCode	= LoginResultCode.BadCredentials;
					Log.LogStatistics(1, "User ({0}) failed logging in with webclient: {1}.", username, messageVersion);
				}
				else
				{
					Log.LogStatistics(1, "User ({0}) logged in with webclient: {1}.", username, messageVersion);
					resultCode	= LoginResultCode.LoginOk;
					cookieName	= System.Web.Security.FormsAuthentication.FormsCookieName;
					string cookieUsername = userId.ToString();
					FormsAuthenticationTicket ticket = persistent 
						? new FormsAuthenticationTicket(
						1, cookieUsername, DateTime.Now, DateTime.Now.AddDays(PersistDays), true, string.Empty)
						: new FormsAuthenticationTicket( cookieUsername, false, timeoutMinutes);
					cookieValue	= System.Web.Security.FormsAuthentication.Encrypt(ticket);

					HttpCookie c = new HttpCookie(FormsAuthentication.FormsCookieName, cookieValue);
					c.Expires = DateTime.Now.AddDays(PersistDays);
					Response.Cookies.Add(c);
				}

				messageVersion	= messageVersion;
			}
			catch
			{
				resultCode		= LoginResultCode.LoginServerError;
				throw;
			}
			finally
			{
				BinaryWriter w	= new BinaryWriter(Response.OutputStream, e);

				w.Write(messageVersion);

				w.Write((System.Int32)resultCode);

				w.Write(userId.ToByteArray());

				w.Write(2*(1+cookieName.Length));
				w.Write(e.GetBytes(cookieName));
				w.Write(new byte[2] {0,0});

				w.Write(2*(1+cookieValue.Length));
				w.Write(e.GetBytes(cookieValue));
				w.Write(new byte[2] {0,0});

				w.Write(2*(1+message.Length));
				w.Write(e.GetBytes(message));
				w.Write(new byte[2] {0,0});

				w.Flush();
			}
		}

		private static void HandleUploadControlUpload(HttpRequest Request, HttpResponse Response, Guid memberId)
		{
			Encoding e		= Encoding.Unicode;
			BinaryReader r	= new BinaryReader(Request.InputStream, e);
			int quota		= 0;
			int usedQuota	= 0;
			int resultCode	= -1;
			string message	= string.Empty;
			Database.UploadResultComplete res = new Database.UploadResultComplete();;
			try
			{
				if(memberId == Guid.Empty)
				{
					resultCode = -2;
					return;
				}

				uint version		= r.ReadUInt32();
				int clientVersion	= 1000+(int)version;


				int		len;
				byte[]	rawText;

				//read hash
				len					= r.ReadInt32();
				byte[] hash			= r.ReadBytes(len);

				//read targetcategory
				Guid targetCategory = new Guid(r.ReadBytes(16));

				//read hintable
				bool hintable		= r.ReadByte()!=0?true:false;

				//read uploadPath
				len					= r.ReadInt32();
				rawText				= r.ReadBytes(len);
				string uploadPath	= e.GetString(rawText, 0, rawText.Length-2);

				//read title
				len					= r.ReadInt32();
				rawText				= r.ReadBytes(len);
				string title		= e.GetString(rawText, 0, rawText.Length-2);

				//read text
				len					= r.ReadInt32();
				rawText				= r.ReadBytes(len);
				string text			= e.GetString(rawText, 0, rawText.Length-2);

				//read path
				len					= r.ReadInt32();
				rawText				= r.ReadBytes(len);
				string path			= e.GetString(rawText, 0, rawText.Length-2);

				len					= r.ReadInt32();

				quota				= Database.GetMemberQuota(memberId, out usedQuota);
				if(quota != -1 && len/1024+usedQuota>quota) 
				{
					resultCode = (int)UploadResult.StoreFull;
					return;
				}

				byte[] imageData	= r.ReadBytes(len);
				if(true)
				{
					if(uploadPath.Length > 0)
						targetCategory = new Guid(uploadPath);

					//TODO! validate title and text
					res = 
						Database.UploadImage(
							memberId, path, "image/jpeg", 
							imageData.Length, new MemoryStream(imageData, false), 
							targetCategory, title, clientVersion, hintable, text
						);
					if(res.ResultCode == UploadResult.Success)
						usedQuota += len/1024;
				}
				else
				{
					FileStream f	= new FileStream(Path.Combine(Configuration.Setting_DataPath,"uploaded"+title+".jpg"), FileMode.Create, FileAccess.ReadWrite);
					f.Write(imageData,0,imageData.Length);
					f.Flush();
					f.Close();
				}
				resultCode = (int)res.ResultCode;

				Log.LogStatistics(1, "User uploaded image with client control.");
			}
			catch(Error_AccessDenied)
			{
				resultCode = 5;
				Log.LogStatistics(1, "User uploaded image with client control, access denied.");
			}
			catch
			{
				Log.LogStatistics(1, "User uploaded image with client control, unhandled exception.");
				resultCode = -1;
				throw;
			}
			finally
			{
				r.Close();
				BinaryWriter w = new BinaryWriter(Response.OutputStream, e);

				w.Write((System.UInt32)1);
				w.Write(resultCode);
				w.Write(res.photoId.ToByteArray());
				w.Write(quota);
				w.Write(usedQuota);

				w.Write((message.Length+1)*2);
				w.Write(e.GetBytes(message));
				w.Write(new byte[2] {0,0});

				w.Flush();
			}
		}

		private class ClientControlsWriter : BinaryWriter
		{
            public readonly byte[] MagicTag = new Guid("{9CEF8854-9CF7-4f19-BE79-989EF60E8C5C}").ToByteArray();
			public Encoding Encoding = Encoding.Unicode;
			public ClientControlsWriter(Stream baseStream)
				:base(baseStream)
			{
				BaseStream.Write(MagicTag, 0, MagicTag.Length);
			}

			public void WriteString(string s)
			{
				if(s == null)
				{
					Write(0);
					return;
				}
				byte[] buffer = this.Encoding.GetBytes(s);
				Write(buffer.Length);
				Write(buffer);
			}
		}

		private class ClientControlsReader : BinaryReader
		{
			public Encoding Encoding = Encoding.Unicode;
			public readonly Guid MagicTag = new Guid("{9CEF8854-9CF7-4f19-BE79-989EF60E8C5C}");
			
			public ClientControlsReader(Stream baseStream)
				:base(baseStream)
			{
			}

			public void ReadTag()
			{
				byte[] buffer = new byte[16];
				Read(buffer, 0, 16);
				if(new Guid(buffer) != MagicTag)
					throw new Exception("Bad network data");
			}

			public string ReadString()
			{
				uint len		= ReadUInt32();
				byte[] buffer	= ReadBytes((int)len);
				string s		= Encoding.GetString(buffer);
				return s;
			}
		}

		public static void HandleUpdate(HttpRequest Request, HttpResponse Response)
		{
			ClientControlsReader r	= new ClientControlsReader(Request.InputStream);
			byte[] buffer;
			FileStream inFile = null;
			try
			{
				Response.ClearContent();
                ClientControlsWriter w		= new ClientControlsWriter(Response.OutputStream);

				int updateManagerVersion	= int.Parse(Request["updateManagerVersion"]);
				int controlReleaseVersion	= int.Parse(Request["controlReleaseVersion"]);
				int processorArchitecture	= int.Parse(Request["processorArchitecture"]);
				bool win95					= Request["platform"] == "95";
				int winMajor				= int.Parse(Request["winmajor"]);
				int winMinor				= int.Parse(Request["winminor"]);
				string winCsd				= Request["winCsd"];
				string gdiplusVerString		= Request["gdiplusver"].Trim();
				bool hasGdiplus				= gdiplusVerString != "0.0";
				bool adminInstall			= bool.Parse(Request["admin"]);
				bool hasMfc71				= bool.Parse(Request["mfc71"]);


				string[] files;
				if(!hasGdiplus)
					files = new string[] { "gdiplus", "minakortcontrols" };
				else
					files = new string[] { "minakortcontrols" };


				//Write version
				w.Write(updateManagerVersion);
				//Write response code
				w.Write(1);
				//Write response message
				string message = "";
				w.WriteString(message);

				//Write file count
				w.Write(files.Length);

				//Write total file size
				int totalSize=0;
				foreach(string file in files)
				{
					string filePath = HttpContext.Current.Server.MapPath(Configuration.RootPath+ "public/" + file + ".dll");
					totalSize += (int)new FileInfo(filePath).Length;
				}
				w.Write(totalSize);

				//Write files
				foreach(string file in files)
				{
					string clientFileName;
					if(file == "minakortcontrols")
					{
						w.Write((byte)(1));
						clientFileName = "minakortcontrols.2.dll";
					}
					else
					{
						w.Write((byte)(0));
						clientFileName = "gdiplus.dll";
					}

					//Write client file name
					w.WriteString(clientFileName);
					

					string filePath = HttpContext.Current.Server.MapPath(Configuration.RootPath+ "public/" + file + ".dll");

					inFile = new FileStream(filePath, FileMode.Open, FileAccess.Read);
					w.Write((int)inFile.Length);
					buffer = new byte[inFile.Length];
					inFile.Read(buffer,0,buffer.Length);
					w.Write(buffer);
					inFile.Close();
					inFile = null;
				}
			}
			finally
			{
				if(inFile != null)
					inFile.Close();
				Response.Flush();
				Response.Close();
				Response.End();
			}
		}

		struct UserInfo
		{
			public Guid id;
			public string username;
			public string email;
			public string name;
		}

		public static void HandleUserLookup(HttpRequest Request, HttpResponse Response, Guid userId)
		{
			try
			{
				ClientControlsReader r	= new ClientControlsReader(Request.InputStream);
				Response.ClearContent();
				ClientControlsWriter w = new ClientControlsWriter(Response.OutputStream);

				w.Write(1);

				string query = Request["userquery"];

				//Write result code
				if(query == null || query.Length == 0) 
				{
					w.Write(-1);
					return;
				}
				else
					w.Write(0);

				query = "%"+query+"%";
				ArrayList data = new ArrayList();
				using(Db db = new Db())
				{
					db.CommandText = @"
							SELECT id, fullNameClean as fullName, username, email 
							FROM tMember 
							WHERE fullName LIKE @q OR email LIKE @q OR username LIKE @q
							ORDER BY fullNameClean ASC	
							";
					db.AddParameter("@q", query);
					while(db.Read())
					{
						UserInfo user	= new UserInfo();
						user.username	= (string)db["username"];
						user.id			= (Guid)db["id"];
						user.email		= db["email"] as string;
						user.name		= (string)db["fullName"];
						data.Add(user);
					}
				}

				w.Write((int)data.Count);
				foreach(object o in data)
				{
					if(o is UserInfo)
					{
						w.Write((byte)0);
						UserInfo user = (UserInfo)o;
						w.Write(user.id.ToByteArray());
						w.WriteString(user.username);
						w.WriteString(user.email);
						w.WriteString(user.name);
					}
				}

				int a = 3;
			}
			finally
			{
				Response.Flush();
				Response.Close();
				Response.End();
			}
		}

		static void HandleCategoryLookup(HttpRequest Request, HttpResponse Response, Guid userId)
		{
			try
			{
				Response.ClearContent();
				ClientControlsWriter w = new ClientControlsWriter(Response.OutputStream);

				//Version
                w.Write((int)1);

				//ResultCode
				if(userId == Guid.Empty)
				{
					w.Write((int)-1);
					return;
				}

				string categoryName = Request["categoryName"];
				if(categoryName == null)
					categoryName = string.Empty;
				categoryName = categoryName.Trim();
				if(!Validation.ValidateCategoryName(categoryName))
				{
					w.Write((int)-2);
					return;
				}

				w.Write((int)0);
				// } result code.

				Database.MemberDetails details = Database.GetMemberDetails(null, userId);
				Guid existingId = Database.GetSubCategory(userId, details.HomeCategoryId, categoryName);
				Database.Category cat = null;
				if(existingId != Guid.Empty)
					cat = Database.GetCategoryInfo(userId, existingId);

				w.Write(existingId != Guid.Empty);
				w.Write(existingId.ToByteArray());
				//canAddPermission
				w.Write(cat != null && cat.CurrentPermission >= Permission.Add);
				//securityPermission
				w.Write(cat != null && cat.CurrentPermission >= Permission.Owner); //TODO: shold be securitypermission.
				w.WriteString(cat != null?cat.Name:categoryName);
				
				//can't send email
				w.Write((byte)0x00);
				//can't share to friends
				w.Write((byte)0x00);

				//Write the permission entries on the category.
				if(existingId == Guid.Empty)
					w.Write(0);
				else
				{
					Guid groupId = Database.GetMemberGroup(userId, "$"+existingId);
					Database.GroupMember[] members = Database.EnumGroupMembers(userId, groupId);
					w.Write((uint)(members.Length-1)); //minus self
					foreach(Database.GroupMember member in members)
					{
						if(member.MemberId == userId)
							continue;
						Database.MemberDetails md = Database.GetMemberDetails(null, member.MemberId);
						w.Write((byte)0);
						w.Write(md.Id.ToByteArray());
						w.WriteString(md.admin_username);
						w.WriteString(md.admin_email);
						w.WriteString(md.Name);

						w.Write((uint)0);
						w.Write((uint)0);
						w.Write((uint)0);
						w.Write((uint)0);
					}
				}
			}
			finally
			{
				Response.Flush();
				Response.Close();
				Response.End();
			}
		}

		static void HandleCreateCategory(HttpRequest Request, HttpResponse Response, Guid userId)
		{
			ClientControlsWriter w = new ClientControlsWriter(Response.OutputStream);
			//-1 = fail, 0=ok, 1=usersFailedAdding
			int result		= -1;
			uint version	= 1;
			Guid id			= Guid.Empty;
			try
			{
				ClientControlsReader r = new ClientControlsReader(Request.InputStream);
				r.ReadTag();

				version					= r.ReadUInt32();
				string name				= r.ReadString();
				string comment			= null;
				id						= new Guid(r.ReadBytes(16));
				bool letFriendsAdd		= r.ReadBoolean();
				bool shareToFriends		= r.ReadBoolean();
				bool sendEmail			= r.ReadBoolean();

				ArrayList members		= new ArrayList();
				uint shareCount			= r.ReadUInt32();
				while(shareCount-- > 0)
				{
					Guid shareId		= new Guid(r.ReadBytes(16));
					members.Add(shareId);
				}

				if(id == Guid.Empty)
				{
					Guid homeCat = Database.GetMemberDetails(null, userId).HomeCategoryId;
					id = Database.GetSubCategory(userId, homeCat, name);
					if(id == Guid.Empty)
						Database.CreatePhotoCategory(userId, homeCat, name, comment, out id);
				}

				if(shareCount==0)
					result = 0;
				else
				{
					Guid groupId = Guid.Empty;
					Database.SetCategoryInheritPermissions(userId, id, false);
					groupId = Database.GetMemberGroup(userId, "$"+id.ToString());
					if(groupId == Guid.Empty)
						//TODO: delete this group when we delete the category.
						Database.CreateMemberGroup(userId, Guid.Empty, "$"+id.ToString(), out groupId);

					try
					{
						Database.ClearGroupMembers(new GuidId(userId), groupId);
						foreach(Guid member in members)
							try
							{
								if(member == userId)
									continue;
								Database.AddUserToMemberGroup(userId, member, groupId, false, GroupMembershipLevel.Default, -1);
							}
							catch
							{
								int a = 3;
							}
						Database.SetGroupPermission(userId, groupId, id, letFriendsAdd?Permission.Add:Permission.Read,-1);

						result = 0;
					}
					catch
					{
						result = 1;
					}
				}

			}
			catch(Exception e)
			{
				string a = e.ToString();
			}
			finally
			{
				w.Write(version);
				w.Write(result);
				w.Write(id.ToByteArray());
			}
		}

	}
}
