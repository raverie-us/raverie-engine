character = ReadCharacter();

switch (character)
{
  case 'a':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'b':
      {
        acceptedToken = false;

        if (DiffString("stract"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Abstract /* Abstract */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 's':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::As /* As */;
        acceptedToken = true;

        character = ReadCharacter();

        switch (character)
        {
          case 's':
          {
            acceptedToken = false;

            if (DiffString("ert"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Assert /* Assert */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'l':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'i':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'a':
              {
                acceptedToken = false;

                if (DiffString("s"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Alias /* Alias */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 'g':
              {
                acceptedToken = false;

                if (DiffString("nof"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Alignof /* Alignof */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

        }
        break;
      }

      case 'n':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'd':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::And /* And */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

          case 'y':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::Any /* Any */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'u':
      {
        acceptedToken = false;

        if (DiffString("to"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Auto /* Auto */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'b':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'a':
      {
        acceptedToken = false;

        if (DiffString("se"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Base /* Base */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'r':
      {
        acceptedToken = false;

        if (DiffString("eak"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Break /* Break */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 's':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 't':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'a':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 't':
              {
                acceptedToken = false;

                if (DiffString("ic"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Static /* Static */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 'c':
              {
                acceptedToken = false;

                if (DiffString("kalloc"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Stackalloc /* Stackalloc */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

          case 'r':
          {
            acceptedToken = false;

            if (DiffString("uct"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Struct /* Struct */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'c':
      {
        acceptedToken = false;

        if (DiffString("ope"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Scope /* Scope */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'i':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'g':
          {
            acceptedToken = false;

            if (DiffString("ned"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Signed /* Signed */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'z':
          {
            acceptedToken = false;

            if (DiffString("eof"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Sizeof /* Sizeof */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'e':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'a':
          {
            acceptedToken = false;

            if (DiffString("led"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Sealed /* Sealed */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 't':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::Set /* Set */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

          case 'n':
          {
            acceptedToken = false;

            if (DiffString("ds"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Sends /* Sends */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'w':
      {
        acceptedToken = false;

        if (DiffString("itch"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Switch /* Switch */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 't':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'r':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'u':
          {
            acceptedToken = false;

            if (DiffString("e"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::True /* True */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'y':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::Try /* Try */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'i':
      {
        acceptedToken = false;

        if (DiffString("meout"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Timeout /* Timeout */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'h':
      {
        acceptedToken = false;

        if (DiffString("row"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Throw /* Throw */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'y':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'p':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'e':
              {
                acceptedToken = false;

                character = ReadCharacter();

                switch (character)
                {
                  case 'i':
                  {
                    acceptedToken = false;

                    if (DiffString("d"))
                    {
                      lastAcceptedPos = this->Position;
                      outToken->TokenId = Grammar::TypeId /* TypeId */;
                      acceptedToken = true;
                    }
                    character = ReadCharacter();

                    break;
                  }

                  case 'n':
                  {
                    acceptedToken = false;

                    if (DiffString("ame"))
                    {
                      lastAcceptedPos = this->Position;
                      outToken->TokenId = Grammar::Typename /* Typename */;
                      acceptedToken = true;
                    }
                    character = ReadCharacter();

                    break;
                  }

                  case 'o':
                  {
                    acceptedToken = false;

                    if (DiffString("f"))
                    {
                      lastAcceptedPos = this->Position;
                      outToken->TokenId = Grammar::Typeof /* Typeof */;
                      acceptedToken = true;
                    }
                    character = ReadCharacter();

                    break;
                  }

                  case 'd':
                  {
                    acceptedToken = false;

                    if (DiffString("ef"))
                    {
                      lastAcceptedPos = this->Position;
                      outToken->TokenId = Grammar::Typedef /* Typedef */;
                      acceptedToken = true;
                    }
                    character = ReadCharacter();

                    break;
                  }

                }
                break;
              }

            }
            break;
          }

        }
        break;
      }

    }
    break;
  }

  case 'r':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'e':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'a':
          {
            acceptedToken = false;

            if (DiffString("donly"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Readonly /* Readonly */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 't':
          {
            acceptedToken = false;

            if (DiffString("urn"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Return /* Return */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'g':
          {
            acceptedToken = false;

            if (DiffString("ister"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Register /* Register */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'f':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::Ref /* Ref */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

          case 'q':
          {
            acceptedToken = false;

            if (DiffString("uire"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Require /* Require */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

    }
    break;
  }

  case 'c':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'a':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 's':
          {
            acceptedToken = false;

            if (DiffString("e"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Case /* Case */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 't':
          {
            acceptedToken = false;

            if (DiffString("ch"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Catch /* Catch */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'l':
      {
        acceptedToken = false;

        if (DiffString("ass"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Class /* Class */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'o':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'n':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 's':
              {
                acceptedToken = false;

                character = ReadCharacter();

                switch (character)
                {
                  case 't':
                  {
                    lastAcceptedPos = this->Position;
                    outToken->TokenId = Grammar::Const /* Const */;
                    acceptedToken = true;

                    character = ReadCharacter();

                    switch (character)
                    {
                      case 'r':
                      {
                        acceptedToken = false;

                        if (DiffString("uctor"))
                        {
                          lastAcceptedPos = this->Position;
                          outToken->TokenId = Grammar::Constructor /* Constructor */;
                          acceptedToken = true;
                        }
                        character = ReadCharacter();

                        break;
                      }

                    }
                    break;
                  }

                }
                break;
              }

              case 't':
              {
                acceptedToken = false;

                if (DiffString("inue"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Continue /* Continue */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

        }
        break;
      }

      case 'h':
      {
        acceptedToken = false;

        if (DiffString("ecked"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Checked /* Checked */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'l':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'o':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'c':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'a':
              {
                acceptedToken = false;

                if (DiffString("l"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Local /* Local */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 'k':
              {
                lastAcceptedPos = this->Position;
                outToken->TokenId = Grammar::Lock /* Lock */;
                acceptedToken = true;

                character = ReadCharacter();

                break;
              }

            }
            break;
          }

          case 'o':
          {
            acceptedToken = false;

            if (DiffString("p"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Loop /* Loop */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

    }
    break;
  }

  case 'i':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 's':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Is /* Is */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case 'n':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::In /* In */;
        acceptedToken = true;

        character = ReadCharacter();

        switch (character)
        {
          case 't':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'e':
              {
                acceptedToken = false;

                character = ReadCharacter();

                switch (character)
                {
                  case 'r':
                  {
                    acceptedToken = false;

                    character = ReadCharacter();

                    switch (character)
                    {
                      case 'n':
                      {
                        acceptedToken = false;

                        if (DiffString("al"))
                        {
                          lastAcceptedPos = this->Position;
                          outToken->TokenId = Grammar::Internal /* Internal */;
                          acceptedToken = true;
                        }
                        character = ReadCharacter();

                        break;
                      }

                      case 'f':
                      {
                        acceptedToken = false;

                        if (DiffString("ace"))
                        {
                          lastAcceptedPos = this->Position;
                          outToken->TokenId = Grammar::Interface /* Interface */;
                          acceptedToken = true;
                        }
                        character = ReadCharacter();

                        break;
                      }

                    }
                    break;
                  }

                }
                break;
              }

            }
            break;
          }

          case 'c':
          {
            acceptedToken = false;

            if (DiffString("lude"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Include /* Include */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'l':
          {
            acceptedToken = false;

            if (DiffString("ine"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Inline /* Inline */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'f':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::If /* If */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case 'm':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'm':
          {
            acceptedToken = false;

            if (DiffString("utable"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Immutable /* Immutable */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'p':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'l':
              {
                acceptedToken = false;

                if (DiffString("icit"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Implicit /* Implicit */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 'o':
              {
                acceptedToken = false;

                if (DiffString("rt"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Import /* Import */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

        }
        break;
      }

    }
    break;
  }

  case 'g':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'l':
      {
        acceptedToken = false;

        if (DiffString("obal"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Global /* Global */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'o':
      {
        acceptedToken = false;

        if (DiffString("to"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Goto /* Goto */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'e':
      {
        acceptedToken = false;

        if (DiffString("t"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Get /* Get */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'n':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'a':
      {
        acceptedToken = false;

        if (DiffString("mespace"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Namespace /* Namespace */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'o':
      {
        acceptedToken = false;

        if (DiffString("t"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Not /* Not */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'e':
      {
        acceptedToken = false;

        if (DiffString("w"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::New /* New */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'u':
      {
        acceptedToken = false;

        if (DiffString("ll"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Null /* Null */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'o':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'r':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Or /* Or */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case 'u':
      {
        acceptedToken = false;

        if (DiffString("t"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Out /* Out */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'p':
      {
        acceptedToken = false;

        if (DiffString("erator"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Operator /* Operator */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'v':
      {
        acceptedToken = false;

        if (DiffString("erride"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Override /* Override */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'f':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'a':
      {
        acceptedToken = false;

        if (DiffString("lse"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::False /* False */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'r':
      {
        acceptedToken = false;

        if (DiffString("iend"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Friend /* Friend */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'l':
      {
        acceptedToken = false;

        if (DiffString("ags"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Flags /* Flags */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'i':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'n':
          {
            acceptedToken = false;

            if (DiffString("ally"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Finally /* Finally */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'x':
          {
            acceptedToken = false;

            if (DiffString("ed"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Fixed /* Fixed */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'o':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'r':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::For /* For */;
            acceptedToken = true;

            character = ReadCharacter();

            switch (character)
            {
              case 'e':
              {
                acceptedToken = false;

                if (DiffString("ach"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::ForEach /* ForEach */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

        }
        break;
      }

      case 'u':
      {
        acceptedToken = false;

        if (DiffString("nction"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Function /* Function */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'e':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'l':
      {
        acceptedToken = false;

        if (DiffString("se"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Else /* Else */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'n':
      {
        acceptedToken = false;

        if (DiffString("um"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Enumeration /* Enumeration */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'x':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 't':
          {
            acceptedToken = false;

            if (DiffString("ern"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Extern /* Extern */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'p':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'l':
              {
                acceptedToken = false;

                if (DiffString("icit"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Explicit /* Explicit */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 'o':
              {
                acceptedToken = false;

                if (DiffString("rt"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Export /* Export */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

        }
        break;
      }

    }
    break;
  }

  case 'u':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 's':
      {
        acceptedToken = false;

        if (DiffString("ing"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Using /* Using */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'n':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 's':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'a':
              {
                acceptedToken = false;

                if (DiffString("fe"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Unsafe /* Unsafe */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 'i':
              {
                acceptedToken = false;

                if (DiffString("gned"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Unsigned /* Unsigned */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

          case 'c':
          {
            acceptedToken = false;

            if (DiffString("hecked"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Unchecked /* Unchecked */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

    }
    break;
  }

  case 'd':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'o':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Do /* Do */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case 'e':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'b':
          {
            acceptedToken = false;

            if (DiffString("ug"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Debug /* Debug */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 's':
          {
            acceptedToken = false;

            if (DiffString("tructor"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Destructor /* Destructor */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'l':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'e':
              {
                acceptedToken = false;

                character = ReadCharacter();

                switch (character)
                {
                  case 't':
                  {
                    acceptedToken = false;

                    if (DiffString("e"))
                    {
                      lastAcceptedPos = this->Position;
                      outToken->TokenId = Grammar::Delete /* Delete */;
                      acceptedToken = true;
                    }
                    character = ReadCharacter();

                    break;
                  }

                  case 'g':
                  {
                    acceptedToken = false;

                    if (DiffString("ate"))
                    {
                      lastAcceptedPos = this->Position;
                      outToken->TokenId = Grammar::Delegate /* Delegate */;
                      acceptedToken = true;
                    }
                    character = ReadCharacter();

                    break;
                  }

                }
                break;
              }

            }
            break;
          }

          case 'f':
          {
            acceptedToken = false;

            if (DiffString("ault"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Default /* Default */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'y':
      {
        acceptedToken = false;

        if (DiffString("namic"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Dynamic /* Dynamic */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'y':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    if (DiffString("ield"))
    {
      lastAcceptedPos = this->Position;
      outToken->TokenId = Grammar::Yield /* Yield */;
      acceptedToken = true;
    }
    character = ReadCharacter();

    break;
  }

  case 'm':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'o':
      {
        acceptedToken = false;

        if (DiffString("dule"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Module /* Module */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'e':
      {
        acceptedToken = false;

        if (DiffString("mberid"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::MemberId /* MemberId */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'u':
      {
        acceptedToken = false;

        if (DiffString("table"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Mutable /* Mutable */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'p':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'a':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'r':
          {
            acceptedToken = false;

            character = ReadCharacter();

            switch (character)
            {
              case 'a':
              {
                acceptedToken = false;

                if (DiffString("ms"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Params /* Params */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

              case 't':
              {
                acceptedToken = false;

                if (DiffString("ial"))
                {
                  lastAcceptedPos = this->Position;
                  outToken->TokenId = Grammar::Partial /* Partial */;
                  acceptedToken = true;
                }
                character = ReadCharacter();

                break;
              }

            }
            break;
          }

          case 'c':
          {
            acceptedToken = false;

            if (DiffString("kage"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Package /* Package */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'r':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'i':
          {
            acceptedToken = false;

            if (DiffString("vate"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Private /* Private */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'o':
          {
            acceptedToken = false;

            if (DiffString("tected"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Protected /* Protected */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case 'o':
      {
        acceptedToken = false;

        if (DiffString("sitional"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Positional /* Positional */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'u':
      {
        acceptedToken = false;

        if (DiffString("blic"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Public /* Public */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'v':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'a':
      {
        acceptedToken = false;

        if (DiffString("r"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Variable /* Variable */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'i':
      {
        acceptedToken = false;

        if (DiffString("rtual"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Virtual /* Virtual */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

      case 'o':
      {
        acceptedToken = false;

        if (DiffString("latile"))
        {
          lastAcceptedPos = this->Position;
          outToken->TokenId = Grammar::Volatile /* Volatile */;
          acceptedToken = true;
        }
        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case 'w':
  {
    tokenType = TokenCategory::Keyword;

    acceptedToken = false;

    character = ReadCharacter();

    switch (character)
    {
      case 'h':
      {
        acceptedToken = false;

        character = ReadCharacter();

        switch (character)
        {
          case 'i':
          {
            acceptedToken = false;

            if (DiffString("le"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::While /* While */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

          case 'e':
          {
            acceptedToken = false;

            if (DiffString("re"))
            {
              lastAcceptedPos = this->Position;
              outToken->TokenId = Grammar::Where /* Where */;
              acceptedToken = true;
            }
            character = ReadCharacter();

            break;
          }

        }
        break;
      }

    }
    break;
  }

  case '.':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Access /* Access */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case '-':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Negative /* Negative, Subtract */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '-':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Decrement /* Decrement */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '>':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::DynamicAccess /* DynamicAccess */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentSubtract /* AssignmentSubtract */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '>':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::GreaterThan /* GreaterThan */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '>':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::BitshiftRight /* BitshiftRight */;
        acceptedToken = true;

        character = ReadCharacter();

        switch (character)
        {
          case '=':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::AssignmentRightShift /* AssignmentRightShift */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

        }
        break;
      }

      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::GreaterThanOrEqualTo /* GreaterThanOrEqualTo */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '~':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BitwiseNot /* BitwiseNot */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '>':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::NonVirtualAccess /* NonVirtualAccess */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case ':':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::TypeSpecifier /* TypeSpecifier, NameSpecifier, Inheritance, InitializerList */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case ',':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::ArgumentSeparator /* ArgumentSeparator */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case '=':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Assignment /* Assignment */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '>':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::RefersTo /* RefersTo */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Equality /* Equality */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '+':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Positive /* Positive, Add */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentAdd /* AssignmentAdd */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '+':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Increment /* Increment */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '/':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Divide /* Divide */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentDivide /* AssignmentDivide */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '/':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::CommentLine /* CommentLine */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '*':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::CommentStart /* CommentStart */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '*':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Multiply /* Multiply, Dereference */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentMultiply /* AssignmentMultiply */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '/':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::CommentEnd /* CommentEnd */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '%':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Modulo /* Modulo */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentModulo /* AssignmentModulo */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '^':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::Exponent /* Exponent */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentExponent /* AssignmentExponent */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '<':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::LessThan /* LessThan */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::LessThanOrEqualTo /* LessThanOrEqualTo */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '<':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::BitshiftLeft /* BitshiftLeft */;
        acceptedToken = true;

        character = ReadCharacter();

        switch (character)
        {
          case '=':
          {
            lastAcceptedPos = this->Position;
            outToken->TokenId = Grammar::AssignmentLeftShift /* AssignmentLeftShift */;
            acceptedToken = true;

            character = ReadCharacter();

            break;
          }

        }
        break;
      }

    }
    break;
  }

  case '$':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BitwiseXor /* BitwiseXor */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentBitwiseXor /* AssignmentBitwiseXor */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '|':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BitwiseOr /* BitwiseOr */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentBitwiseOr /* AssignmentBitwiseOr */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '|':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::LogicalOr /* LogicalOr */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '&':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BitwiseAnd /* BitwiseAnd, AddressOf */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::AssignmentBitwiseAnd /* AssignmentBitwiseAnd */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

      case '&':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::LogicalAnd /* LogicalAnd */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '!':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::LogicalNot /* LogicalNot */;
    acceptedToken = true;

    character = ReadCharacter();

    switch (character)
    {
      case '=':
      {
        lastAcceptedPos = this->Position;
        outToken->TokenId = Grammar::Inequality /* Inequality */;
        acceptedToken = true;

        character = ReadCharacter();

        break;
      }

    }
    break;
  }

  case '@':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::PropertyDelegate /* PropertyDelegate */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case ';':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::StatementSeparator /* StatementSeparator */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case '[':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BeginIndex /* BeginIndex, BeginTemplate, BeginAttribute, OldBeginInitializer */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case ']':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::EndIndex /* EndIndex, EndTemplate, EndAttribute, OldEndInitializer */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case '(':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BeginFunctionCall /* BeginFunctionCall, BeginFunctionParameters, BeginGroup */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case ')':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::EndFunctionCall /* EndFunctionCall, EndFunctionParameters, EndGroup */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case '{':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::BeginScope /* BeginScope, BeginInitializer */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

  case '}':
  {
    tokenType = TokenCategory::Symbol;

    lastAcceptedPos = this->Position;
    outToken->TokenId = Grammar::EndScope /* EndScope, EndInitializer */;
    acceptedToken = true;

    character = ReadCharacter();

    break;
  }

}
